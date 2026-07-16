# Backend and Engine Parity Gaps — colibrì

**Date:** 2026-07-16
**Updated:** 2026-07-17 00:00 (converter analysis)

This document is the single authoritative reference for the colibrì backend and engine stack. It reflects the actual state of `main` after PRs #46 and #47, and incorporates the converter analysis.

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

**Converter:** `c/tools/convert_qwen35_safetensors.py`

**Output format:**
- **Quantized tensors** (attention weights, embed, lm_head, shared experts, dense MLP):
  - Main tensor: `dtype=U8`, shape `[O, I]`, contains packed int8 or int4 values
  - Scale tensor: `{name}.qs`, `dtype=F32`, shape `[O]`, contains per-row scales
- **F32 tensors** (norms, routers, biases, linear attention projections, A_log, dt_bias):
  - Single tensor: `dtype=F32`, shape as-is

**Quantization rules** (`should_quantize()`):
- `.mlp.experts.*` → int4
- `model.embed_tokens.*`, `lm_head.*`, `.self_attn.*` (non-linear_attn), `.shared_expert.*`, dense MLP → int8
- Everything else → F32

**Engine's `load_tensor_f32()` handles U8 payload** (`dtype == 3`):
- Case 1: `packed_bytes == nelems` → int8 (1 byte/element)
- Case 2: `packed_bytes == out_dim * bytes_per_row` → int4 (2 values/byte)
- Case 3: Ornith doubled int8 layout (special case for legacy models)

The engine dequantizes U8 payloads back to F32 in memory using the .qs scales.

---

## Current State Summary

### Backends (all four)

| Backend | File | Status | What it does |
|---------|------|--------|--------------|
| **CPU** | `c/backend_runtime.c` (lane 0) | ✅ Working | Full CPU matmul with OpenMP parallelism, supports fmt 0/1/2/3 |
| **NPU** | `c/backend_npu.c` | ✅ Host-backed shim | Copies to host memory, uses `matmul_host()`, supports fmt 0/1/2/3 |
| **Vulkan** | `c/backend_vulkan.c` | ✅ Host-backed shim | Copies to host memory, probes Vulkan loader, uses `matmul_host()`, supports fmt 0/1/2/3 |
| **ROCm/HIP** | `c/backend_rocm.hip` | ✅ HIP shim | Full HIP implementation with unified memory, `quant_matmul` kernel supports fmt 0/1/2/3 |

All backends share the `coli_cuda_*` ABI and dispatch through `backend_runtime.c`. When a selected backend fails upload or matmul, the runtime falls back to the CPU path.

**Key observation:** All four backends already support fmt 0 (F32), fmt 1 (INT8), fmt 2 (INT4), and fmt 3 (INT2). The `valid_fmt()` function in every backend accepts all four. The user's decision to only use fmt 0/1/2 means **no backend changes are needed for format support** — they already handle it.

### Engines

| Engine | File | Status |
|--------|------|--------|
| **GLM-5.2** | `c/glm.c` | ✅ Complete (int4/int8 resident, expert streaming from disk, LRU cache, profiling, IDOT kernels) |
| **Qwen3.5 MoE** | `c/qwen35_moe.c` | ⚠️ Skeleton forward pass (lazy expert loading, basic attention, no GPU backend integration, loads everything as F32) |

### Format Usage by Engine

**GLM-5.2** (`c/glm.c`):
- `fmt=0` (F32): norms (`final_norm`, `enorm`, `hnorm`, `mtp_norm`), routers (`router`, `router_bias`), biases (`la_in_proj_a`, `la_in_proj_b`), `lm_head` (when io_bits >= 8), `embed` (when io_bits >= 8), MTP layers
- `fmt=1` (INT8): dense weights, shared experts, `lm_head` (when io_bits < 8), `embed` (when io_bits < 8)
- `fmt=2` (INT4): dense resident weights (main compression path, ~8.7 GB for 17B params)

**Qwen3.5 MoE** (`c/qwen35_moe.c`):
- **All tensors loaded as F32** via `load_tensor_f32()` — no quantization path yet
- Norms (`in_ln`, `post_ln`, `la_norm`), routers (`router`), biases (`la_dt_bias`), `q_norm`, `k_norm`, `la_in_proj_a/b/qkv/z`, `embed`, `lm_head`, `final_norm` → all F32
- Expert weights (gate_proj, up_proj, down_proj) → loaded as F32, no quantization
- **Gap:** The converter outputs int4/int8 + F32 scales, but the engine dequantizes everything back to F32 in memory. This means:
  - No memory savings from quantization (tensors are stored as F32 after dequantization)
  - No use of the backend runtime (all matmuls are CPU)
  - No use of INT8/INT4 formats in the backend

---

## Backend Implementation Assessment

### The good news: all backends already support the needed formats

Every backend (`backend_runtime.c`, `backend_npu.c`, `backend_vulkan.c`, `backend_rocm.hip`) has:
- `valid_fmt()` accepting fmt 0, 1, 2, 3
- `packed_bytes()` computing correct sizes for all formats
- `scale_bytes()` handling scale tensors for non-F32 formats
- `host_matmul()` (NPU/Vulkan) or `weight_at()` (ROCm) decoding all formats correctly
- `decode_i4()` and `decode_i2()` for packed int4/int2

**No backend code changes are needed for format support.** The formats are already implemented.

### The real work: GPU-native kernels for NPU and Vulkan

The NPU and Vulkan backends currently use `matmul_host()` which is a **CPU fallback**. They copy weights to host memory and run CPU matmul. This works but doesn't use the GPU.

**ROCm already has a real GPU kernel** (`quant_matmul` in `backend_rocm.hip`) that runs on the RDNA 4 iGPU. It supports fmt 0/1/2/3 with unified memory (Strix Halo APU).

**Vulkan needs a compute shader** for the same `quant_matmul` kernel. This is the only backend that requires new low-level code (SPIR-V shader pipeline, descriptor setup, command recording).

**NPU (XDNA 2) needs XRT kernels** for fixed-shape subgraphs. This is the lowest priority and most complex.

### Approach comparison

| Backend | Current state | What's needed | Complexity |
|---------|--------------|---------------|------------|
| **CPU** | ✅ Working | Nothing | Done |
| **ROCm** | ✅ Real HIP kernel | Nothing for int4/int8/F32. FP8/FP6/FP4 would need new kernel types (but user said no) | Done |
| **Vulkan** | ⚠️ Host-backed shim | SPIR-V compute shader for `quant_matmul` (fmt 0/1/2), VkBuffer/VkDescriptorSet management, command recording | Medium — needs low-level Vulkan API code |
| **NPU** | ⚠️ Host-backed shim | XRT kernel loading, fixed-shape subgraph offload, device discovery | High — needs XRT/XCLBIN toolchain |

**Recommendation:** Focus on Vulkan first (mature ecosystem, Mesa/RADV available on Fedora), then ROCm optimization (already works, just needs validation), then NPU (lowest priority, most complex).

---

## Key Gaps

1. **Qwen3.5 engine needs to store tensors in quantized format** — Currently dequantizes everything to F32 in memory. Needs to:
   - Store tensors in INT8/INT4 format (fmt 1/2) in the `QLayer` struct
   - Use the backend runtime for matmul (which supports fmt 1/2)
   - Only dequantize at matmul time (dequant-on-use)

2. **Vulkan GPU-native kernels** — Need SPIR-V compute shader for `quant_matmul` (fmt 0/1/2), VkBuffer/VkDescriptorSet management, command recording. This is the only backend requiring new low-level code.

3. **NPU GPU-native kernels** — Need XRT kernel loading, fixed-shape subgraph offload. Lowest priority.

4. **Backend-specific correctness tests** — Every backend needs tests for the same paths CPU exercises.

5. **Wire planner/doctor/server reporting** — Consistent backend availability reporting.

6. **Qwen3.5 feature completeness** — Full attention math, multimodal, grammar decoding, logprobs.

---

## Implementation Priority

1. **Qwen3.5 engine quantization storage** — Store tensors in INT8/INT4 format (fmt 1/2) in the `QLayer` struct, use backend runtime for matmul, dequant-on-use at matmul time. This unlocks memory savings and GPU acceleration.

2. **Vulkan GPU-native kernels** — SPIR-V compute shader for `quant_matmul` (fmt 0/1/2), VkBuffer/VkDescriptorSet management, command recording. This is the only backend requiring new low-level code.

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
