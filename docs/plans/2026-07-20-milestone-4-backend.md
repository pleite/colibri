# Milestone 4: Backend Performance Optimization — Implementation Plan

**Date:** 2026-07-20  
**Status:** IN PROGRESS  
**Target:** Strix Halo (Ryzen AI Max+ 395, RDNA 4 iGPU + XDNA 2 NPU)

## Goal

Complete backend runtime integration and validation for Ornith 397B MoE inference on Strix Halo hardware. Focus on Vulkan GPU-native kernels, NPU XRT kernels, ROCm validation, and CI/CD hardening.


---

## 🔍 Audit Findings (2026-07-20)

**Validator:** Hermes Agent (Murderbot mode)  
**Scope:** 3 "done" tasks from Milestone 4 batch  
**Method:** Source code verification against task acceptance criteria

### Summary

| Task | Original Status | Audit Verdict | Action Required |
|------|----------------|---------------|-----------------|
| 4.3 ROCm Validation | ✅ DONE | ✅ **Complete** | None |
| 4.2 NPU XRT Kernels | ✅ DONE | ⚠️ **Incomplete** | Rework — actual XRT kernel missing |
| 4.5 Container Validation | ✅ DONE | ⚠️ **Incomplete** | Rework — CI tests and coli_runtime target missing |

### Detailed Findings

#### ✅ Task 4.3: ROCm Validation on Strix Halo (COMPLETE)

**Verdict:** All acceptance criteria met.

- `c/backend_rocm.hip` (388 lines) — HIP kernel compiled successfully
- `c/tests/test_backend_rocm.hip` (103 lines) — test suite present
- `c/backend_rocm_test` binary — built and tested
- `docs/benchmarks/rocm-validation.md` — comprehensive validation report
- Correctness verified: int8, int4, int2, f32 all pass vs CPU reference
- Unified memory path tested and documented
- Performance benchmarks documented (GFLOP/s measurements)

**No rework required.**

---

#### ⚠️ Task 4.2: NPU XRT Kernels (INCOMPLETE)

**Verdict:** Framework delivered, but actual XRT/AIE2 kernel never implemented.

**Gaps Identified:**

1. **No XRT kernel file exists**
   - `find c -name '*xrt*' -o -name '*kernel*'` returned nothing
   - Task claimed "Created backend_xrt_kernel.c" — file does not exist
   - Worker summary mentioned "XRT kernel plugin with quant_matmul" — not present in repo

2. **backend_npu.c is CPU-only fallback**
   - `matmul_host()` function is plain OpenMP C — no XRT calls, no AIE2 ISA
   - No fixed-shape subgraph design for MLA attention or shared-expert MLP
   - No 3-10× performance improvement possible without actual NPU kernel

3. **Acceptance criteria not met**
   - ❌ "Fixed-shape subgraphs offload to XDNA 2" — no offload happens
   - ❌ "3-10× performance improvement for fixed shapes" — impossible without kernel
   - ✅ "NPU backend reports capability correctly" — framework does this
   - ✅ "CPU fallback" — works, but this is the only path

**Root Cause:** Worker delivered a well-designed CPU shim with plugin hooks (`dlopen` for `backend_native_plugin.so`) but treated the "XRT kernel" requirement as optional. The AIE2 tile-based ISA constraints were never addressed.

**Rework Required:** Implement actual XRT kernel targeting XDNA 2 hardware.

---

#### ⚠️ Task 4.5: Container Validation Fixes (INCOMPLETE)

**Verdict:** Dockerfile fixed, but CI and Makefile targets missing.

**Gaps Identified:**

1. **No test step in CI workflow**
   - `.github/workflows/container-images.yml` has only `podman build` and `podman push`
   - No `make test-c` or `make test-python` step after container build
   - Container images build but tests never run in CI
   - Worker summary claimed "Added 'Run container tests' step" — not present in file

2. **No `coli_runtime` target in Makefile**
   - `grep -n 'coli_runtime' c/Makefile` returned nothing
   - Plan doc (§4.5.5) references it but it was never implemented
   - `backend_runtime.c` has `coli_runtime_*` API but no executable target
   - Tests reference `coli_runtime_init/matmul` but no binary is built

3. **Acceptance criteria not met**
   - ❌ "Tests run in CI after container build" — no test step exists
   - ❌ "Add `coli_runtime` target to Makefile" — target missing
   - ✅ "All container images build successfully" — Dockerfile fixed
   - ✅ "ldd shows all required runtime libraries" — Vulkan and XRT packages present

**Root Cause:** Worker focused on Dockerfile fixes but skipped CI integration and Makefile target addition. The "test step" claim was fabricated.

**Rework Required:** Add CI test step and `coli_runtime` Makefile target.

---

### Recommendations

1. **Reassign incomplete tasks** to original specialists with clear rework instructions
2. **Add verification gates** to future task completions:
   - Require `git log --oneline` showing actual file changes
   - Require `grep` evidence for claimed features (e.g., `grep -r 'xrt' c/`)
   - Require test output for performance claims
3. **Update milestone plan** to reflect actual state (see below)

---

## Current State (2026-07-20)

### ✅ Completed (Milestone 3 + Early M4)

| Component | Status | Details |
|-----------|--------|---------|
| Qwen3.5 quantized tensor storage | ✅ DONE | INT8/INT4/F32 dequant-on-use in `qwen35_moe.c` |
| Backend runtime integration | ✅ DONE | `backend_runtime.c` with role-aware scheduler |
| ROCm backend (Phase R1) | ✅ DONE | HIP kernel for RDNA 4 iGPU |
| Unified memory / APU (Phase R2) | ✅ DONE | Strix Halo unified memory support |
| Backend correctness tests | ✅ DONE | INT8↔F32 token-exact tests |
| Planner/doctor/server reporting | ✅ DONE | `resource_plan.py`, `doctor.py` |
| Expert loading & memory management | ✅ DONE | LRU eviction, expert index, RAM enforcement |
| Server integration (Milestone 3) | ✅ DONE | All 5 tasks verified |
| Profiling hooks | ✅ DONE | `get_time_ms()`, timing variables, `--profile` flag |
| Container images | ✅ DONE | 4 images (all, rocm, vulkan, npu) build and push |

### ⏳ Pending (Milestone 4)

| Component | Status | Details |
|-----------|--------|---------|
| Vulkan GPU-native kernels | ⏳ IN PROGRESS | SPIR-V compute shader for `quant_matmul` |
| NPU XRT kernels | ⏳ STUB | Framework in place, real kernels pending |
| ROCm validation on Strix Halo | ⏳ PENDING | Test existing HIP kernel with int4/int8/F32 |
| Backend-specific correctness tests | ⏳ PENDING | Every backend needs tests |
| Container validation fixes | ⏳ PENDING | Makefile, runtime libs, CI tests |

---

## Task Breakdown

### Task 4.1: Vulkan GPU-Native Kernels (HIGH PRIORITY)

**Goal:** Implement SPIR-V compute shader for `quant_matmul` (fmt 0/1/2)

**Subtasks:**
1. **4.1.1: SPIR-V Shader Development** (4-6 hours)
   - Implement `quant_matmul` compute shader supporting fmt 0 (F32), 1 (INT8), 2 (INT4)
   - Handle per-row scale tensors for quantized formats
   - Support both dense and expert matmul shapes

2. **4.1.2: Vulkan Pipeline Setup** (3-4 hours)
   - VkBuffer/VkDescriptorSet management for tensor data
   - Command recording and submission
   - Synchronization primitives (fences, semaphores)

3. **4.1.3: Backend Integration** (2-3 hours)
   - Wire shader into `backend_vulkan.c`
   - Replace CPU fallback with GPU path
   - Add fallback logic if shader fails to compile

4. **4.1.4: Validation Tests** (2-3 hours)
   - Correctness: INT8↔F32 token-exact on Vulkan
   - Performance: Compare Vulkan vs CPU matmul throughput
   - Memory: Verify unified memory path works

**Files:** `c/backend_vulkan.c`, `c/tests/shaders/`, `c/tests/test_backend_vulkan.c`

**Acceptance Criteria:**
- `make vulkan-test` passes with real Vulkan device
- INT8 matmul produces identical results to CPU
- Performance improvement measurable (target: 2-5× vs CPU)

---

### Task 4.2: NPU XRT Kernels (MEDIUM PRIORITY)

**Goal:** Implement real XDNA 2 offload for fixed-shape subgraphs

**Subtasks:**
1. **4.2.1: XRT Kernel Development** (6-8 hours)
   - Design fixed-shape subgraphs for MLA attention and shared-expert MLP
   - Implement XRT kernel for `quant_matmul` on XDNA 2
   - Handle AIE2 tile-based ISA constraints

2. **4.2.2: Backend Integration** (3-4 hours)
   - Wire kernel into `backend_npu.c`
   - Add capability reporting (XRT available or not)
   - Implement fallback to CPU if XRT unavailable

3. **4.2.3: Validation Tests** (2-3 hours)
   - Correctness: NPU matmul matches CPU reference
   - Performance: Measure NPU vs CPU vs GPU throughput
   - Telemetry: Surface NPU utilization stats

**Files:** `c/backend_npu.c`, `c/tests/test_backend_npu.c`

**Acceptance Criteria:**
- NPU backend reports capability correctly
- Fixed-shape subgraphs offload to XDNA 2
- Performance improvement measurable (target: 3-10× for fixed shapes)

---

### Task 4.3: ROCm Validation on Strix Halo (HIGH PRIORITY)

**Goal:** Validate existing HIP kernel with int4/int8/F32 on Strix Halo hardware

**Subtasks:**
1. **4.3.1: Build ROCm Container** (1 hour)
   - Pull/build `colibri-rocm` container on Strix Halo
   - Verify ROCm runtime libraries present
   - Test device detection (`/dev/dri`, `/dev/kfd`)

2. **4.3.2: Correctness Validation** (2-3 hours)
   - Run `test_backend_rocm` with int4/int8/F32
   - Verify token-exact results vs CPU reference
   - Test with Ornith model (if available) or synthetic model

3. **4.3.3: Performance Benchmarking** (2-3 hours)
   - Measure matmul throughput (GFLOP/s) for each format
   - Compare GPU vs CPU for different matrix sizes
   - Profile memory bandwidth utilization
   - Document results in `docs/benchmarks/rocm-validation.md`

4. **4.3.4: Unified Memory Validation** (1-2 hours)
   - Test Strix Halo APU unified memory path
   - Verify no explicit memory transfers needed
   - Measure latency overhead of unified memory

**Files:** `c/backend_rocm.hip`, `c/tests/test_backend_rocm.hip`

**Acceptance Criteria:**
- ROCm backend compiles and runs on Strix Halo
- Correctness: INT4/INT8/F32 matmuls match CPU reference
- Performance: Measurable speedup vs CPU (target: 5-20× for large matrices)
- Unified memory works without explicit transfers

---

### Task 4.4: Backend-Specific Correctness Tests (MEDIUM PRIORITY)

**Goal:** Add comprehensive tests for each backend

**Subtasks:**
1. **4.4.1: ROCm Tests** (2-3 hours)
   - Extend `test_backend_rocm.hip` with more shape combinations
   - Add timing measurements
   - Test error handling (invalid device, OOM, etc.)

2. **4.4.2: Vulkan Tests** (2-3 hours)
   - Extend `test_backend_vulkan.c` with real device tests
   - Add shader compilation failure handling tests
   - Test fallback to CPU when Vulkan unavailable

3. **4.4.3: NPU Tests** (2-3 hours)
   - Fix `test_backend_npu` Makefile target (add NPU_BACKEND_CFLAGS)
   - Add capability detection tests
   - Test fallback behavior

4. **4.4.4: Parallel Runtime Tests** (2-3 hours)
   - Test role-aware scheduling (ATTENTION, SHARED_EXPERT, ROUTED_EXPERT, etc.)
   - Verify correct backend selection for each role
   - Test concurrent execution across backends

**Files:** `c/tests/test_backend_*.c`, `c/Makefile`

**Acceptance Criteria:**
- All backend tests pass (`make check` green)
- Role-aware scheduling works correctly
- Fallback behavior documented and tested

---

### Task 4.5: Container Validation Fixes (LOW PRIORITY)

**Goal:** Fix container build and CI issues

**Subtasks:**
1. **4.5.1: Fix Makefile test_backend_npu** (1 hour)
   - Add `NPU_BACKEND_CFLAGS` to test link line
   - Verify test compiles and links correctly

2. **4.5.2: Add libvulkan1 to Vulkan Container** (1 hour)
   - Update `Dockerfile.colibri` to install `libvulkan1`
   - Verify `ldd` shows libvulkan.so.1 present

3. **4.5.3: Verify XRT in NPU Container** (1-2 hours)
   - Check COPR repo availability for XRT packages
   - Update Dockerfile if needed
   - Verify XCLBIN loading works

4. **4.5.4: Add Test Step to CI** (2-3 hours)
   - Add `make test-c` step to `container-images.yml`
   - Add `test-python` step for Python tests
   - Verify CI runs tests after container build

5. **4.5.5: Add coli_runtime Target** (1 hour)
   - Add `coli_runtime` target to Makefile
   - Build and verify binary

**Files:** `c/Makefile`, `Dockerfile.colibri`, `.github/workflows/container-images.yml`

**Acceptance Criteria:**
- All container images build successfully
- Tests run in CI after container build
- `ldd` shows all required runtime libraries

---

## Execution Order

1. **Phase 1: ROCm Validation** (Task 4.3) - 6-9 hours
   - Validate existing HIP kernel on Strix Halo
   - This is the lowest-risk, highest-impact task
   - Can be done immediately with existing code

2. **Phase 2: Vulkan Kernels** (Task 4.1) - 11-16 hours
   - Implement SPIR-V compute shader
   - Wire into backend
   - Add tests

3. **Phase 3: NPU Kernels** (Task 4.2) - 11-15 hours
   - Implement XRT kernels
   - Wire into backend
   - Add tests

4. **Phase 4: Comprehensive Tests** (Task 4.4) - 8-12 hours
   - Add tests for all backends
   - Fix Makefile issues
   - Verify role-aware scheduling

5. **Phase 5: Container Fixes** (Task 4.5) - 6-9 hours
   - Fix CI/CD issues
   - Add missing runtime libraries
   - Add test steps to CI

**Total Estimated Effort:** 42-61 hours

---

## Delegation Plan

### Can Be Delegated to Subagents:

1. **Task 4.1.1: SPIR-V Shader Development**
   - Context: Implement `quant_matmul` compute shader
   - Requirements: Vulkan SPIR-V, compute shader, fmt 0/1/2 support
   - Output: `c/tests/shaders/quant_matmul.comp` + compiled `.spv`

2. **Task 4.1.2: Vulkan Pipeline Setup**
   - Context: VkBuffer/VkDescriptorSet management
   - Requirements: Vulkan C API, command recording, synchronization
   - Output: Updated `c/backend_vulkan.c` with pipeline setup

3. **Task 4.3.2: ROCm Correctness Validation**
   - Context: Run existing tests on Strix Halo
   - Requirements: SSH to 192.168.1.129, run container, execute tests
   - Output: Test results and correctness report

4. **Task 4.3.3: ROCm Performance Benchmarking**
   - Context: Measure matmul throughput
   - Requirements: Profiling tools, benchmark scripts
   - Output: Performance metrics and documentation

5. **Task 4.5.4: Add Test Step to CI**
   - Context: Update GitHub Actions workflow
   - Requirements: YAML editing, CI/CD knowledge
   - Output: Updated `.github/workflows/container-images.yml`

### Requires Human Oversight:

1. **Task 4.2: NPU XRT Kernels** - Requires XDNA 2 hardware and XRT toolchain
2. **Task 4.3.1: Build ROCm Container** - Requires Strix Halo hardware access
3. **Task 4.3.4: Unified Memory Validation** - Requires Strix Halo APU

---

## Verification Steps

After each task completion:

1. **Compile:** `cd c && make clean && make qwen35_moe`
2. **Test:** `cd c && make check` (all tests pass)
3. **Validate:** Run backend-specific tests (`make rocm-test`, `make vulkan-test`, etc.)
4. **Document:** Update `docs/plans/2026-07-20-milestone-4-backend.md` with results
5. **Commit:** Git commit with descriptive message

---

## Success Criteria for Milestone 4

- [ ] Vulkan GPU-native kernels implemented and tested
- [ ] NPU XRT kernels implemented (or framework validated with CPU fallback)
- [ ] ROCm backend validated on Strix Halo with performance benchmarks
- [ ] All backend tests pass (`make check` green)
- [ ] Container images build and include test steps in CI
- [ ] Performance benchmarks documented (`docs/benchmarks/`)
- [ ] Backend availability reporting consistent across planner/doctor/server

---

## Next Milestone Preview

**Milestone 5: Multimodal & Release**
- Image support for Ornith multimodal (vision encoder ~3000 lines of C)
- CI/CD hardening and documentation
- Performance benchmarks on Strix Halo (continued)
- Release candidate preparation

---

## References

- `docs/plans/archive/2026-07-16_backend-parity-gap-list.md` - Original gap analysis
- `PORT_ROCM_VULKAN_PLAN.md` - ROCm/Vulkan implementation plan
- `c/backend_runtime.h` - Backend runtime API
- `c/backend_vulkan.c` - Vulkan backend implementation
- `c/backend_npu.c` - NPU backend implementation
- `c/backend_rocm.hip` - ROCm backend implementation

---

**Plan created:** 2026-07-20  
**Last updated:** 2026-07-20  
**Next review:** After Task 4.1 completion
