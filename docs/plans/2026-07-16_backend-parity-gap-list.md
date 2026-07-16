# Backend and Engine Parity Gaps — colibrì

**Date:** 2026-07-16
**Updated:** 2026-07-16 23:30 (format decision: int4 + int8 + F32 only)

This document is the single authoritative reference for the colibrì backend and engine stack. It reflects the actual state of `main` after PRs #46 and #47, and incorporates the user's format decision.

---

## Format Decision

**Only three tensor formats are needed:**

| fmt | Name | Use case | Where |
|-----|------|----------|-------|
| 0 | F32 | Norms, routers, biases, lm_head, embed, q_norm/k_norm, linear attention projections, MTP layers | All engines |
| 1 | INT8 | Dense weights, shared experts, per-row scales (dequant-on-use) | GLM-5.2 |
| 2 | INT4 | Dense resident weights (main compression path, 0.5 byte/param) | GLM-5.2 |

**No FP8/FP6/FP4.** The models are converted to int4/int8. F32 is only for the small sensitive tensors (norms, routers, biases) that must stay at full precision. No quantization or dequantization for other FP formats.

**Qwen3.5 MoE** currently loads everything as F32 via `load_tensor_f32()`. No quantization path yet — this is part of the remaining work.

---

## What Was Implemented (PR #46 + #47)

### PR #46 — Backend runtime fallback semantics

- `c/backend_runtime.c`: Full shared runtime layer with:
  - Backend mask resolution via `COLI_RUNTIME_ENGINES` / `COLI_RUNTIME_DISABLE_ENGINES` env vars
  - Chunked dispatch across multiple backends (lane splitting)
  - CPU fallback for every backend (NPU, Vulkan, ROCm, CUDA)
  - Tensor caching with FNV-1a hash (input hash → cached output)
  - `host_memory_info()` for all backends (real sysconf, not hardcoded stubs)
- `c/backend_npu.c`: Host-backed shim — `matmul_host()` with OpenMP parallelism, real memory reporting
- `c/backend_vulkan.c`: Host-backed shim — `matmul_host()` with OpenMP parallelism, Vulkan loader probing, real memory reporting
- `c/backend_runtime.o`: Compiled and linked
- `c/tests/test_backend_runtime.c`: Tests for the new runtime
- `c/backend_vulkan.o`: Compiled and linked

### PR #47 — Qwen3.5 MoE engine hardening

- `c/qwen35_moe.c`: Lazy expert loading (`ensure_expert()`), `min_p` sampling parameter, int2 format removal (only int4/int8 supported now)
- `c/qwen35_moe`: Updated binary
- `c/openai_server.py`: Added `min_p` parameter support
- `c/tests/test_qwen35_moe.c` + binary: Updated tests
- `c/tests/test_resource_plan.py`: Updated tests
- `c/resource_plan.py`: Updated resource planning
- `docs/plans/2026-07-16_backend-parity-gap-list.md`: This file

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
| **Qwen3.5 MoE** | `c/qwen35_moe.c` | ⚠️ Skeleton forward pass (lazy expert loading, basic attention, no GPU backend integration, no quantization) |

### Format Usage by Engine

**GLM-5.2** (`c/glm.c`):
- `fmt=0` (F32): norms (`final_norm`, `enorm`, `hnorm`, `mtp_norm`), routers (`router`, `router_bias`), biases (`la_in_proj_a`, `la_in_proj_b`), `lm_head` (when io_bits >= 8), `embed` (when io_bits >= 8), MTP layers
- `fmt=1` (INT8): dense weights, shared experts, `lm_head` (when io_bits < 8), `embed` (when io_bits < 8)
- `fmt=2` (INT4): dense resident weights (main compression path, ~8.7 GB for 17B params)
- `fmt=3` (INT2): removed in PR #47

**Qwen3.5 MoE** (`c/qwen35_moe.c`):
- All tensors loaded as F32 via `load_tensor_f32()` — no quantization path yet
- Norms (`in_ln`, `post_ln`, `la_norm`), routers (`router`), biases (`la_dt_bias`), `q_norm`, `k_norm`, `la_in_proj_a/b/qkv/z`, `embed`, `lm_head`, `final_norm` → all F32
- Expert weights (gate_proj, up_proj, down_proj) → loaded as F32, no quantization

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
| **Vulkan** | ⚠️ Host-backed shim | SPIR-V compute shader for `quant_matmul`, VkBuffer/VkDescriptorSet management, command recording | Medium — needs low-level Vulkan API code |
| **NPU** | ⚠️ Host-backed shim | XRT kernel loading, fixed-shape subgraph offload, device discovery | High — needs XRT/XCLBIN toolchain |

**Recommendation:** Focus on Vulkan first (mature ecosystem, Mesa/RADV available on Fedora), then ROCm optimization (already works, just needs validation), then NPU (lowest priority, most complex).

---

## Key Gaps

1. **Vulkan GPU-native kernels** — Need SPIR-V compute shader for `quant_matmul` (fmt 0/1/2), VkBuffer/VkDescriptorSet management, command recording. This is the only backend requiring new low-level code.
2. **NPU GPU-native kernels** — Need XRT kernel loading, fixed-shape subgraph offload. Lowest priority.
3. **Qwen3.5 backend integration** — `qwen35_moe.c` does NOT use the backend runtime. All matmuls are CPU.
4. **Qwen3.5 quantization** — Currently loads everything as F32. Needs int4/int8 path for expert weights (like GLM).
5. **Backend-specific correctness tests** — Every backend needs tests for the same paths CPU exercises.
6. **Wire planner/doctor/server reporting** — Consistent backend availability reporting.
7. **Qwen3.5 feature completeness** — Full attention math, multimodal, grammar decoding, logprobs.

---

## Implementation Priority

1. **Vulkan GPU-native kernels** — SPIR-V compute shader for `quant_matmul` (fmt 0/1/2), VkBuffer/VkDescriptorSet management, command recording. This is the only backend requiring new low-level code.
2. **ROCm validation** — Test existing HIP kernel with int4/int8/F32 on Strix Halo, validate unified memory path.
3. **Integrate Qwen3.5 with backend runtime** — Wire `qwen35_moe.c` to use `coli_runtime_matmul()` for expert matmuls.
4. **Qwen3.5 quantization path** — Add int4/int8 quantization for expert weights (gate_proj, up_proj, down_proj), matching GLM's `qt_alloc`/`qt_fill`/`qt_matvec` pattern.
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
