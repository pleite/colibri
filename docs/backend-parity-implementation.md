# Qwen3.5 Backend Parity — Implementation Notes

**Date:** 2026-07-16

This document describes the changes that bring the Qwen3.5 MoE engine up to parity
with the GLM-5.2 engine, as tracked in
[docs/plans/2026-07-16_backend-parity-gap-list.md](plans/2026-07-16_backend-parity-gap-list.md).

It covers four areas:

1. Quantized tensor storage for Qwen3.5 (memory savings + dequant-on-use).
2. A parallel, role-aware multi-backend scheduler in the C runtime.
3. Planner and doctor support for simultaneous backends with device-role affinity.
4. OpenAI-compatible server support for model Jinja chat templates.

All of the work below is validated by `cd c && make check` (portable CPU build +
C and Python tests).

---

## 1. Qwen3.5 quantized tensor storage

### Motivation

Previously `qwen35_moe.c` loaded every weight through `load_tensor_f32()`, which
**dequantized INT8/INT4 payloads back to F32 in memory**. This defeated the whole
point of the quantized checkpoints: a weight that ships as 1 byte/param (INT8) or
0.5 byte/param (INT4) was expanded to 4 bytes/param at load time. GLM-5.2 instead
keeps weights in their compact form and dequantizes *on use*.

### The `QTensor` container

A new `QTensor` struct (mirroring GLM's layout) stores a weight in its native
format:

```c
typedef struct {
    int fmt;              /* 0 = F32, 1 = INT8, 2 = INT4 */
    int O;                /* output rows   */
    int I;                /* input columns */
    void *data;           /* packed payload: F32[O*I], int8[O*I], or int4[O*ceil(I/2)] */
    float *scales;        /* per-output-row F32 scale (fmt != 0) */
    ColiCudaTensor *handle; /* backend-runtime cache handle for resident tensors */
} QTensor;
```

All matmul weights in `QLayer` (attention q/k/v/o, dense MLP, router, shared
expert, and routed experts) are now `QTensor`. The small, precision-sensitive
tensors (norms, biases, `shared_expert_gate`, and the linear-attention
projections `la_in_proj_qkv` / `la_in_proj_z`) remain plain F32 `float*`, exactly
as the gap-list specifies.

### Loading: `load_qtensor()`

`load_qtensor()` preserves the native quantized payload when the checkpoint stores
it in a **standard row-major packed layout with matching per-row scales**:

- `dtype == U8`, `nbytes == O*I`, scale rows `== O` → **fmt 1 (INT8)**, payload +
  scales kept verbatim.
- `dtype == U8`, `nbytes == O*ceil(I/2)`, scale rows `== O` → **fmt 2 (INT4)**,
  nibble-packed payload + scales kept verbatim.
- Everything else — genuine F32 tensors, missing scales, and the irregular
  "Ornith doubled/expanded int8" layouts — falls back to a dequantized **fmt 0**
  buffer via the existing, well-tested `load_tensor_f32()` decoder.

Missing tensors keep the identity/default `QTensor` installed by
`init_layer_defaults()`, preserving the historical missing-weight behavior.

### Compute: `matmul_qt()` and `qwen_cpu_qmatmul()`

`qwen_cpu_qmatmul()` computes `y[o] = scale[o] * Σ_i x[i] * dequant(w[o,i])` with a
single trailing scale multiply. For fmt 0 this is **bit-identical** to the old
`matmul_vec`, so F32 models remain token-exact; the INT8/INT4 paths mirror the
exact dequant math in `load_tensor_f32` (`int8: val*scale`, `int4: (nibble-8)*scale`).
No F32 expansion and no weight copy happen, which is where the memory savings come
from.

`matmul_qt(QTensor *w, x, out, role)` is the single entry point used by the
forward pass:

- On a **CPU-only build** (or when no accelerator lane is active) it calls
  `qwen_cpu_qmatmul` directly, reading straight from the packed `QTensor.data`
  with **no copy** — this is what preserves the memory savings.
- When an accelerator lane (NPU/Vulkan/GPU) is active it routes the op through
  `coli_runtime_matmul_ex(...)`, the parallel role-aware scheduler (section 2),
  which can offload and split the output across CPU/NPU/GPU lanes.

### Memory savings

Because the payloads stay packed and resident:

| Format | Bytes/param | vs F32 |
|--------|-------------|--------|
| F32 (fmt 0) | 4 | — |
| INT8 (fmt 1) | 1 | 75% smaller |
| INT4 (fmt 2) | 0.5 | 87.5% smaller |

This matches the ~57% aggregate reduction targeted for Qwen3.5 in the gap-list
(dense weights INT4, shared experts INT8, routed experts INT4 streaming). Routed
experts continue to **stream in lazily** (`ensure_expert`) and now stay resident
in quantized form.

### Correctness test

`c/tests/test_qwen35_moe.c` adds a token-exact equivalence test: it builds one
model whose `q_proj` is stored as native INT8 (diag(2), per-row scale 0.05) and an
otherwise-identical model whose `q_proj` is the equivalent F32 `diag(0.1)`, then
asserts both produce **identical output tokens**. This proves the memory-saving
dequant-on-use path does not change results. The existing token-exact F32 tests
also continue to pass, now flowing through `matmul_qt` → `qwen_cpu_qmatmul` (fmt 0).

---

## 2. Parallel, role-aware multi-backend scheduler

### What changed

`backend_runtime.c` previously dispatched its "lanes" **sequentially**. The runtime
now runs lanes **in parallel with pthreads** and partitions the matmul output rows
across lanes according to an **operator role**.

### Operator roles

`backend_runtime.h` defines `coli_op_role`:

```
COLI_OP_ROLE_AUTO
COLI_OP_ROLE_ATTENTION
COLI_OP_ROLE_SHARED_EXPERT
COLI_OP_ROLE_ROUTED_EXPERT
COLI_OP_ROLE_DENSE
COLI_OP_ROLE_SMALL
```

Each role maps to a lane-affinity weighting (`g_role_affinity`). The output rows of
a matmul are partitioned across the active lanes according to that weighting, with
**CPU always retained as a fallback lane** and the remainder routed to the
highest-weight lane. This lets NPU-preferred "sensor" ops (attention, router,
shared expert) and GPU-preferred throughput ops (routed experts, dense MLP) run
**simultaneously** on different devices for the same token.

### New public API

- `int coli_runtime_matmul_ex(tensor, y, x, weights, scales, fmt, S, I, O, device, role)`
  — role-aware matmul. `coli_runtime_matmul(...)` is now a thin wrapper that calls
  it with `COLI_OP_ROLE_AUTO`.
- `int coli_runtime_backend_mask(void)` — bitmask of active backends
  (`COLI_RUNTIME_BACKEND_{CPU,NPU,VULKAN,CUDA}_BIT`).
- `int coli_runtime_parallel_enabled(void)` — whether the parallel scheduler is on.
- `int coli_runtime_active_backends(char *buf, size_t n)` — human-readable list of
  active backends.

### Configuration

- `COLI_RUNTIME_PARALLEL=1` enables the parallel scheduler.
- `COLI_RUNTIME_ENGINES=<list>` advertises which engines may run simultaneously.
- `COLI_ROLE_<ROLE>=<backend>` pins a role to a device class.

These are emitted automatically by the planner (section 3).

### Test

`c/tests/test_backend_parallel.c` builds the runtime with a real second lane
(`-DCOLI_ENABLE_NPU`, a pure-CPU shim needing no hardware) and validates parallel
dispatch, role partitioning, and the reporting API.

---

## 3. Vulkan native plugin path

The Vulkan backend now exposes an optional native-plugin dispatch path for real
compute-kernel implementations. The default shim still preserves the CPU reference
path as a safe fallback, but when `COLI_VULKAN_KERNEL_LIB=/path/to/lib.so` is set
it will try to load a plugin exporting `coli_vulkan_native_init()`,
`coli_vulkan_native_shutdown()`, and `coli_vulkan_native_matmul()`.

To make that path concrete, the repository now includes a reference Vulkan compute
shader (`c/tests/shaders/shader.comp`) and a matching SPIR-V asset
(`c/tests/shaders/comp.spv`) modeled after the
`bmilde/vulkan_matrix_mul` example. `c/tests/backend_vulkan_native_plugin.c`
implements an opt-in plugin that uses that shader to dispatch a simple matmul
through Vulkan when the runtime is available; if the shader, device, or driver is
unavailable, it falls back to the existing CPU matmul implementation so the system
continues to run.

---

## 4. Planner and doctor: simultaneous backends with role affinity

### `resource_plan.py`

`build_plan(..., backend="parallel")` now:

- Selects the set of engines to run at once via `_select_parallel_backends()` — the
  fastest detected GPU stack (cuda→rocm→vulkan) **plus** the NPU when present (CPU
  is always an implicit lane).
- Computes a per-role device mapping via `_role_affinity_for()` using
  `DEFAULT_ROLE_AFFINITY`: NPUs are preferred for latency-sensitive, low
  arithmetic-intensity "sensor" paths (attention, router = `small`, shared expert),
  GPUs for throughput-bound routed experts and dense MLP, CPU as the universal
  fallback.
- Records `parallel`, `parallel_backends`, and `role_affinity` in the plan's
  `accelerator` block.

`environment_for_plan()` emits `COLI_RUNTIME_PARALLEL=1`,
`COLI_RUNTIME_ENGINES=<engines>`, and `COLI_ROLE_<ROLE>=<backend>` so the C runtime
picks up the same plan. If `backend="parallel"` is requested with no GPU/NPU
detected, the plan records a warning and falls back to CPU.

### `doctor.py`

When a plan is parallel, doctor adds an `accelerator.parallel` check reporting the
active engines and the resolved role affinity.

---

## 5. OpenAI server: model Jinja chat templates

`openai_server.py` gains optional support for a model's own
`chat_template.jinja` (the HuggingFace-style template shipped with Qwen3.5/GLM
checkpoints), which matters for exact fidelity with agent harnesses like Hermes:

- `load_jinja_chat_template(model_dir)` reads `chat_template.jinja` if present.
- `render_chat_jinja(template_source, messages, ...)` renders it, exposing the
  standard `messages`, `tools`, `add_generation_prompt`, and `enable_thinking`
  variables (the `apply_chat_template` contract). `jinja2` is imported **lazily**,
  so the server stays dependency-free unless a template is actually used; template
  errors surface as clean `400` API errors.
- Activation is **opt-in** via `COLI_JINJA_CHAT=1`, so the existing built-in
  GLM/Qwen text renderers (and their golden tool-calling tests) remain the default.

The server already supported OpenAI-compatible tools (GLM + Qwen tool-call
formats), SSE streaming, multimodal content parts, per-request KV-cache slots
(`KV_SLOTS` / `cache_slot`), and logprobs; those are unchanged.

### KV cache and the Hermes agent harness

The engine exposes multiple KV-cache slots (`KV_SLOTS`, default 1, max 16). Each
chat/completions request may target a slot via the `cache_slot` field, letting an
agent harness keep several concurrent conversation contexts warm and reuse the
prefix KV across turns. Combine with `COLI_JINJA_CHAT=1` to render Hermes's exact
tool/template formatting while reusing cached KV for efficient multi-turn tool
loops.

---

## Validation

```sh
cd c && make check
```

runs the portable CPU build plus all C tests (`test_qwen35_moe`,
`test_backend_parallel`, `test_backend_runtime`, grammar, json, safetensors,
tier) and the Python test suite (`test_resource_plan`, `test_doctor`,
`test_openai_server`, converters, …).

### Not covered here (hardware-gated)

Real GPU-native Vulkan SPIR-V and NPU XRT kernels still fall back to CPU: they
cannot be compiled or validated in a CPU-only CI sandbox. The runtime lane and
role-affinity wiring is in place so a real kernel can be dropped into the existing
lane without further scheduler changes.

To make that drop-in path explicit, the Vulkan and NPU shims now accept an
optional native-kernel plugin via environment variables:

- `COLI_VULKAN_KERNEL_LIB=/path/to/libcolibri_vulkan_kernel.so`
- `COLI_NPU_XRT_LIB=/path/to/libcolibri_npu_xrt.so` (preferred for AMD XDNA/XRT)
- `COLI_NPU_KERNEL_LIB=/path/to/libcolibri_npu_kernel.so` (fallback compatibility name)

A plugin may export `coli_vulkan_native_matmul()` / `coli_npu_native_matmul()`
(and optional init/shutdown hooks) and will be used in preference to the CPU
reference path; otherwise the existing host fallback remains active.
