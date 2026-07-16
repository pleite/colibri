# Backend and Engine Parity Gaps — colibrì

**Date:** 2026-07-16
**Updated:** 2026-07-16 23:00 (post-PR #46, #47 audit)

This document consolidates everything from the previous gap analyses and plan files into a single authoritative reference. It reflects the actual state of `main` after PRs #46 and #47 were merged.

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
- `c/tests/test_backend_runtime` (binary): Removed (was a leftover test binary)
- `c/backend_vulkan.o`: Compiled and linked

### PR #47 — Qwen3.5 MoE engine hardening

- `c/qwen35_moe.c`: Lazy expert loading (`ensure_expert()`), `min_p` sampling parameter, int2 format removal (only int4/int8 supported now)
- `c/qwen35_moe`: Updated binary
- `c/openai_server.py`: Added `min_p` parameter support
- `c/tests/test_qwen35_moe.c` + binary: Updated tests
- `c/tests/test_resource_plan.py`: Updated tests
- `c/resource_plan.py`: Updated resource planning
- `docs/plans/2026-07-16_backend-parity-gap-list.md`: This file

### PR #45 — Original gap analysis (still relevant context)

- `docs/plans/backend-gap-analysis.md`: Detailed per-backend gap analysis
- `docs/plans/2026-07-15_engine-capability-audit.md`: Engine feature audit
- `docs/plans/2026-07-15_copilot-ornith-feature-completeness.md`: Ornith FP8 feature completeness plan

---

## Current State Summary

### Backends (all four)

| Backend | File | Status | What it does |
|---------|------|--------|--------------|
| **CPU** | `c/backend_runtime.c` (lane 0) | ✅ Working | Full CPU matmul with OpenMP parallelism |
| **NPU** | `c/backend_npu.c` | ✅ Host-backed shim | Copies to host memory, uses `matmul_host()` |
| **Vulkan** | `c/backend_vulkan.c` | ✅ Host-backed shim | Copies to host memory, probes Vulkan loader, uses `matmul_host()` |
| **ROCm/HIP** | `c/backend_rocm.hip` | ✅ HIP shim | Full HIP implementation with unified memory support for Strix Halo APU |

All backends share the `coli_cuda_*` ABI and dispatch through `backend_runtime.c`. When a selected backend fails upload or matmul, the runtime falls back to the CPU path.

### Engines

| Engine | File | Status |
|--------|------|--------|
| **GLM-5.2** | `c/glm.c` | ✅ Complete (int4/int8 resident, expert streaming from disk, LRU cache, profiling) |
| **Qwen3.5 MoE** | `c/qwen35_moe.c` | ⚠️ Skeleton forward pass (lazy expert loading, basic attention, no GPU backend integration) |

### Key Gaps

1. **GPU-native kernels** — NPU and Vulkan backends fall back to CPU `matmul_host()`. ROCm has real HIP kernels but the full execution stack (FP8/FP6/FP4 resident format, kernel dispatch) is not yet integrated.
2. **Qwen3.5 backend integration** — `qwen35_moe.c` does NOT use the backend runtime. All matmuls are CPU.
3. **Low-bit resident tensor format** — FP8/FP6/FP4 pipeline not implemented end-to-end.
4. **Multimodal** — Vision, video, audio completely missing from Qwen3.5 engine.
5. **Grammar-constrained decoding** — `grammar.h` exists but not integrated.
6. **Logprobs/embeddings** — Engine discards logits after argmax; embeddings endpoint returns 501.
7. **KV-cache persistence** — Cache is in-memory only.

---

## Implementation Priority

1. **Finish the shared backend contract** — Already mostly done. The runtime fallback semantics are in place.
2. **Implement low-bit resident tensor path** — FP8/FP6/FP4 pipeline for GPU backends.
3. **Integrate Qwen3.5 with backend runtime** — Wire `qwen35_moe.c` to use `coli_runtime_matmul()`.
4. **Add backend-specific correctness tests** — Every backend needs tests for the same paths CPU exercises.
5. **Wire planner/doctor/server reporting** — Consistent backend availability reporting.
6. **Qwen3.5 feature completeness** — Full attention math, multimodal, grammar decoding, logprobs.
7. **Performance tuning** — GPU-native kernels for NPU/Vulkan, ROCm FP8 native arithmetic.

---

## Obsolete Files (to remove)

- `PORT_QWEN35_MOE_PLAN.md` — Superseded by this consolidated document
- `PORT_ROCM_VULKAN_PLAN.md` — Superseded by this consolidated document
- `docs/plans/2026-07-15_copilot-ornith-feature-completeness.md` — Content absorbed into this doc
- `docs/plans/2026-07-15_engine-capability-audit.md` — Content absorbed into this doc
- `docs/plans/backend-gap-analysis.md` — Content absorbed into this doc
- `docs/plans/2026-07-16_backend-parity-gap-list.md` — This IS the consolidation
