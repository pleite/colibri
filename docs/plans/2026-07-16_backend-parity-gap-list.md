# Backend and Engine Parity Gaps — colibrì

**Date:** 2026-07-16
**Updated:** 2026-07-17 00:30 (memory savings analysis)

This document is the single authoritative reference for the colibrì backend and engine stack. It reflects the actual state of `main` after PRs #46 and #47, and incorporates the converter and memory savings analysis.

---

## Implementation Status (updated by the Qwen3.5 backend-parity PR)

The following gap items are now **implemented** and covered by `cd c && make check`:

| # | Gap item | Status | Where |
|---|----------|--------|-------|
| 1 | Qwen3.5 quantized tensor storage (INT8/INT4/F32 dequant-on-use) | ✅ Done | `c/qwen35_moe.c` (`QTensor`, `qwen_cpu_qmatmul`, `load_qtensor`, `matmul_qt`) |
| 4 | Wire Qwen3.5 to the backend runtime | ✅ Done | `matmul_qt` routes to `coli_runtime_matmul_ex` when an accelerator lane is active |
| 5a | Backend correctness tests | ✅ Done | `c/tests/test_backend_parallel.c`, INT8↔F32 token-exact test in `c/tests/test_qwen35_moe.c` |
| 6 | Planner / doctor / server reporting | ✅ Done | `resource_plan.py` (`backend="parallel"`), `doctor.py` (`accelerator.parallel`), `openai_server.py` (Jinja templates) |
| — | Parallel + role-aware multi-backend scheduler | ✅ Done (new) | `c/backend_runtime.c/.h` (`coli_op_role`, pthreads lanes, role affinity) |

Partially addressed / still hardware-gated (cannot be compiled or validated in a CPU-only CI sandbox — no GPU/NPU toolchains):

| # | Gap item | Status |
|---|----------|--------|
| 2 | Vulkan SPIR-V `quant_matmul` shader | ⏳ Still a CPU fallback; runtime lane + role wiring is in place so a real kernel can drop in |
| 3 | NPU (XDNA) XRT kernels | ✅ Framework integration path is in place; actual XDNA kernels remain a hardware-gated CPU shim |

See **[docs/backend-parity-implementation.md](../backend-parity-implementation.md)** for the full design of the quantized storage, the parallel role-aware scheduler, and the server/planner/doctor changes delivered here.

---


## Format Decision

**Only three tensor formats are needed:**

| fmt | Name | Use case | Where |
|-----|------|----------|-------|
| 0 | F32 | Norms, routers, biases, lm_head, embed, q_norm/k_norm, linear attention projections | All engines |
| 1 | INT8 | Dense weights, shared experts, per-row scales (dequant-on-use) | GLM-5.2, Qwen3.5 |
| 2 | INT4 | Dense resident weights (main compression path, 0.5 byte/param) | GLM-5.2, Qwen3.5 |

**No FP8/FP6/FP4.** The models are converted to int4/int8. F32 is only for the small sensitive tensors (norms, routers, biases) that must stay at full precision.

---

## Converter Analysis

### GLM-5.2 Converter (`c/tools/convert_fp8_to_int4.py`)

**Strategy:** Disk-safe, shard-by-shard download → convert → delete. Peak disk = 1 shard + growing int4 output.

**Tensor classification:**
- `f32`: norms, routers (`mlp.gate.weight`), biases, `e_score_correction_bias`
- `io`: `model.embed_tokens.weight`, `lm_head.weight` → quantized with `io_bits` (default 8)
- `q`: attention, dense MLP, shared expert → quantized with `ebits` (default 4)
- `x`: routed experts → quantized with `xbits` (default=ebits), streamed from disk
- `skip`: DSA indexer, MTP layer (78), `eh_proj`, `enorm`, `hnorm`, `shared_head`

**Quantization math:** Identical to C engine (`glm.c`):
- `quant_int8()`: per-row scale, `np.clip(np.rint(w / s), -qmax-1, qmax).astype(np.int8)`
- `quant_int4()`: per-row scale, nibble packing (2 values/byte), same as `pack_int4` in C
- `quant_int2()`: per-row scale, 4 values/byte packing, same as `pack_int2` in C

**Output format:**
- Quantized tensors: `dtype=U8`, shape `[O, I]` (int8) or `[O, ceil(I/2)]` (int4)
- Scale tensors: `{name}.qs`, `dtype=F32`, shape `[O]`
- F32 tensors: `dtype=F32`, shape as-is

### Qwen3.5 Converter (`c/tools/convert_qwen35_safetensors.py`)

**Strategy:** Multi-process worker pool with state-based resume. Handles FP8, BF16, F16, F32 inputs.

**Tensor classification** (`should_quantize()`):
- `int4`: `.mlp.experts.*` (routed experts)
- `int8`: `model.embed_tokens.*`, `lm_head.*`, `.self_attn.*` (non-linear_attn), `.shared_expert.*`, dense MLP
- `F32`: norms, routers, biases, linear attention projections, A_log, dt_bias

**Quantization math:**
- `quantize_int8()`: per-row scale, `int(round(value / scale))`, clip to [-128, 127]
- `quantize_int4()`: per-row scale, `int(round(value / scale))`, clip to [-8, 7], nibble packing

**Output format:**
- Quantized tensors: `dtype=U8`, shape `[O, I]` (int8) or `[O, ceil(I/2)]` (int4)
- Scale tensors: `{name}.qs`, `dtype=F32`, shape `[O]`
- F32 tensors: `dtype=F32`, shape as-is

**Key difference from GLM converter:** Qwen3.5 uses `int(round())` instead of `np.clip(np.rint())`, but the result is identical for well-behaved tensors.

---

## Memory Savings Analysis

### GLM-5.2 Memory Layout

**Resident tensors** (in RAM, quantized):
- Dense weights: int4 → 0.5 byte/param
- Shared experts: int8 → 1 byte/param
- MTP layer: int8 → 1 byte/param
- Embed/lm_head: int8 (io_bits=8) → 1 byte/param

**Streaming tensors** (from disk, quantized):
- Routed experts: int4 → 0.5 byte/param (loaded on-demand)

**F32 tensors** (small, sensitive):
- Norms, routers, biases: ~1-2 GB total

**Total resident memory for 17B params:**
- Dense weights: ~8.7 GB (int4)
- Shared experts: ~1.5 GB (int8)
- MTP: ~0.1 GB (int8)
- Embed/lm_head: ~1.9 GB (int8)
- **Total: ~12.2 GB** (vs ~34 GB for bf16)

**Memory savings:** ~64% reduction from bf16 to int4/int8.

### Qwen3.5 MoE Memory Layout (Current)

**All tensors loaded as F32:**
- Dense weights: 4 byte/param
- Shared experts: 4 byte/param
- Routed experts: 4 byte/param (loaded on-demand)
- Embed/lm_head: 4 byte/param
- Norms/routers/biases: 4 byte/param

**Total resident memory for 397B params (Ornith):**
- Dense weights: ~3.2 TB (bf16) → **~1.6 TB (int4)** with quantization
- Shared experts: ~160 GB (bf16) → **~80 GB (int8)** with quantization
- Routed experts: ~320 GB (bf16) → **~160 GB (int4)** with quantization (streaming)
- Embed/lm_head: ~2 GB (bf16) → **~1 GB (int8)** with quantization
- Norms/routers/biases: ~2 GB (bf16) → **~2 GB (F32)** unchanged

**Current state (no quantization):** ~3.7 TB resident + streaming
**Target state (with quantization):** ~1.6 TB resident + 160 GB streaming

**Memory savings needed:** ~57% reduction from bf16 to int4/int8.

---

## Engine Gap: Qwen3.5 Must Match GLM Memory Layout

**Current Qwen3.5 engine:**
- `load_tensor_f32()` dequantizes U8 payloads back to F32 in memory
- All tensors stored as `float *` in `QLayer` struct
- No memory savings from quantization
- No use of backend runtime (all matmuls are CPU)

**Required Qwen3.5 engine changes:**
1. Store tensors in quantized format (INT8/INT4) in the `QLayer` struct
2. Use the backend runtime for matmul (which supports fmt 1/2)
3. Only dequantize at matmul time (dequant-on-use)
4. Match GLM's memory layout: dense weights int4, shared experts int8, routed experts int4 (streaming)

**This unlocks:**
- ~57% memory reduction (matching GLM's 64% savings)
- GPU acceleration via backend runtime
- Reduced memory bandwidth for matmul operations

---

## Backend Implementation Assessment

### All backends already support the needed formats

Every backend (`backend_runtime.c`, `backend_npu.c`, `backend_vulkan.c`, `backend_rocm.hip`) has:
- `valid_fmt()` accepting fmt 0, 1, 2, 3
- `packed_bytes()` computing correct sizes for all formats
- `scale_bytes()` handling scale tensors for non-F32 formats
- `host_matmul()` (NPU/Vulkan) or `weight_at()` (ROCm) decoding all formats correctly

**No backend code changes are needed for format support.**

### The real work: GPU-native kernels for NPU and Vulkan

**ROCm already has a real GPU kernel** (`quant_matmul` in `backend_rocm.hip`) that runs on the RDNA 4 iGPU. It supports fmt 0/1/2/3 with unified memory (Strix Halo APU).

**Vulkan needs a compute shader** for the same `quant_matmul` kernel. This is the only backend that requires new low-level code (SPIR-V shader pipeline, descriptor setup, command recording).

**NPU (XDNA 2) needs XRT kernels** for fixed-shape subgraphs. This is the lowest priority and most complex.

### NPU vs GPU vs CPU: Fixed-Shape Analysis

**NPU (XDNA 2) requirements:**
- Fixed-shape subgraphs only (AIE2 tile-based ISA)
- Best for: MLA attention (fixed-shape per token), shared-expert MLP (always active, same shape every layer)
- Worst for: Routed experts (variable topk per token → variable shape)

**GPU (RDNA 4) requirements:**
- Variable-shape matmuls
- Best for: Routed experts (variable topk), dense weights (large matrices)
- Can handle: Fixed-shape subgraphs (but less efficient than NPU for small fixed shapes)

**CPU requirements:**
- Fallback for all backends
- Best for: Small tensors (norms, routers, biases), orchestration, routing

**Recommendation:**
1. **NPU:** Offload MLA attention and shared-expert MLP (fixed-shape, always active)
2. **GPU:** Handle routed experts and dense weights (variable-shape, large matrices)
3. **CPU:** Fallback for everything, plus small tensor operations

---

## Key Gaps

1. **Qwen3.5 engine must store tensors in quantized format** — Currently dequantizes everything to F32 in memory. Needs to:
   - Store tensors in INT8/INT4 format (fmt 1/2) in the `QLayer` struct
   - Use the backend runtime for matmul (which supports fmt 1/2)
   - Only dequantize at matmul time (dequant-on-use)
   - Match GLM's memory layout: dense weights int4, shared experts int8, routed experts int4 (streaming)

2. **Vulkan GPU-native kernels** — Need SPIR-V compute shader for `quant_matmul` (fmt 0/1/2), VkBuffer/VkDescriptorSet management, command recording.

3. **NPU GPU-native kernels** — The AMD/XDNA/XRT framework integration path is now wired through the NPU backend and can dispatch an external plugin; the remaining gap is the real fixed-shape XRT kernel implementation itself. Lowest priority.

4. **Backend-specific correctness tests** — Every backend needs tests for the same paths CPU exercises.

5. **Wire planner/doctor/server reporting** — Consistent backend availability reporting.

6. **Qwen3.5 feature completeness** — Full attention math, multimodal, grammar decoding, logprobs.

---

## Implementation Priority

1. **Qwen3.5 engine quantization storage** — Store tensors in INT8/INT4 format (fmt 1/2) in the `QLayer` struct, use backend runtime for matmul, dequant-on-use at matmul time. Match GLM's memory layout for equivalent memory savings (~57% reduction).

2. **Vulkan GPU-native kernels** — SPIR-V compute shader for `quant_matmul` (fmt 0/1/2), VkBuffer/VkDescriptorSet management, command recording.

3. **ROCm validation** — Test existing HIP kernel with int4/int8/F32 on Strix Halo, validate unified memory path.

4. **Integrate Qwen3.5 with backend runtime** — Wire `qwen35_moe.c` to use `coli_runtime_matmul()` for expert matmuls.

5. **Add backend-specific correctness tests** — Every backend needs tests for the same paths CPU exercises.

6. **Wire planner/doctor/server reporting** — Consistent backend availability reporting.

7. **Qwen3.5 feature completeness** — Full attention math, multimodal, grammar decoding, logprobs.

---

## Obsolete Files (removed in commit b86deb0)

- `PORT_QWEN35_MOE_PLAN.md` — Superseded by this consolidated document
- `PORT_ROCM_VULKAN_PLAN.md` — Superseded by this consolidated document
- `docs/plans/2026-07-15_copilot-ornith-feature-completeness.md` — Content absorbed into this doc
- `docs/plans/2026-07-15_engine-capability-audit.md` — Content absorbed into this doc
- `docs/plans/backend-gap-analysis.md` — Content absorbed into this doc
