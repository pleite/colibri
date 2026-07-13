# ROCm · Vulkan · NPU plan — colibrì

Tracks GPU/accelerator support for **AMD hardware on Fedora 43**, targeting the
**AMD Strix Halo** APU (Ryzen AI Max+ series — RDNA 4 Radeon 890M iGPU + XDNA 2
NPU, 128 GB LPDDR5X unified memory).

All accelerator code follows the same pattern as the CUDA backend: a thin
abstraction layer in `c/backend_*.{hip,c}` exports the same `coli_cuda_*` API,
so `glm.c` and the rest of the engine stay unchanged across backends.

---

## Phase R1 — ROCm/HIP backend (✅ complete)

Goal: run resident-tensor GPU matmuls on AMD GPUs via ROCm/HIP — functionally
identical to the CUDA path.

| item | detail |
|---|---|
| source | `c/backend_rocm.hip`, `c/backend_rocm.h` |
| API | identical to `backend_cuda.h` — same `coli_cuda_*` symbol names |
| build flag | `ROCM=1` (requires ROCm ≥ 6.0; `hipcc` at `/opt/rocm/bin/hipcc`) |
| compiler | `hipcc` replaces `nvcc`; CUDA → HIP API is a 1-to-1 translation |
| glm.c change | `#ifdef COLI_ROCM` selects `backend_rocm.h` over `backend_cuda.h`; all `#ifdef COLI_CUDA` paths are reused |
| test | `make rocm-test ROCM=1` runs `tests/test_backend_rocm.hip` |

**Build (Fedora 43):**

```bash
# install ROCm (official amdgpu-install or community COPR — see §Fedora notes)
sudo dnf install rocm-hip-sdk rocm-hip-devel hipcc

cd c
make ROCM=1                     # build glm with ROCm backend
make rocm-test ROCM=1           # correctness test (q8/q4/q2/f32)
COLI_CUDA=1 COLI_GPU=0 SNAP=/nvme/glm52_i4 ./glm 64 4 4
```

The environment variables are the same as CUDA: `COLI_CUDA=1 COLI_GPU=<n>`,
`CUDA_EXPERT_GB=<gb>`, etc.

**Multi-GPU:**
```bash
COLI_CUDA=1 COLI_GPUS=0,1 CUDA_EXPERT_GB=32 PIN=stats.txt SNAP=... ./glm 64 4 4
```

---

## Phase R2 — Unified memory / APU (✅ complete)

Goal: zero-copy tensor access on AMD APUs where the GPU and CPU share the same
physical DRAM (Strix Halo Radeon 890M, Hawk Point, Phoenix, …).

| item | detail |
|---|---|
| detection | `hipDeviceAttributeIntegrated` in `coli_cuda_init`; runtime override via `COLI_ROCM_UNIFIED=1/0` |
| tensor storage | `hipHostMalloc(hipHostMallocMapped)` → host pointer + GPU device pointer from `hipHostGetDevicePointer`; no DMA copy |
| scratch buffers | same mapped host memory path for x/y scratch to avoid VRAM carve-out limit |
| resource planner | `discover_rocm_gpus()` sets `unified_memory=True` when total VRAM < 4 GB (heuristic for APU carve-out); `build_plan()` uses available RAM as VRAM budget; `environment_for_plan()` sets `COLI_ROCM_UNIFIED=1` |

**Strix Halo (Radeon 890M) expected config:**

```bash
# 128 GB unified: all pinned experts share the same physical DRAM as RAM cache.
# GPU tier = GPU RDNA 4 computes the hot experts; no copy overhead.
COLI_CUDA=1 COLI_GPU=0 COLI_ROCM_UNIFIED=1 \
CUDA_EXPERT_GB=48 PIN=stats.txt PIN_GB=128 \
SNAP=/nvme/glm52_i4 ./glm 64 4 8
```

`coli plan --accel rocm --gpu 0 --ram 120` automatically discovers unified
memory and reports the correct budget.

---

## Phase V1 — Vulkan compute backend (execution plan)

Goal: a Vulkan compute path (`c/backend_vulkan.c`, `c/backend_vulkan.h`) that
works with the **Mesa** open-source driver stack — no proprietary runtime.

Scope:
- GLSL compute shaders for q8/q4/q2/f32 matmul (same kernel semantics as
  `backend_rocm.hip :: quant_matmul`)
- Compiled offline to SPIR-V with `glslc`/`glslangValidator`, embedded as
  `uint32_t` arrays in the C source
- `VkBuffer` + `VkDescriptorSet` management via the Vulkan C API
- Same `coli_cuda_*` ABI as CUDA/ROCm backends
- Build flag: `VULKAN=1`; links `-lvulkan`

Vulkan is the right backend for Strix Halo when ROCm is unavailable (e.g. Mesa
RADV without the ROCm userspace), and is the only option for Vulkan-only iGPUs
(Intel Arc, Qualcomm Adreno).

**Required toolchain (Fedora 43):**
```bash
sudo dnf install vulkan-devel vulkan-tools glslc spirv-tools mesa-vulkan-drivers
```

**Target API:**
- `VULKAN=1` → `make glm VULKAN=1`
- `COLI_ACCEL=vulkan COLI_ACCEL_DEVICES=0`
- `coli plan --accel vulkan --gpu 0`

### V1 phased execution (Mesa/RADV-first)

#### V1.1 — Backend scaffolding and build wiring
- Add `c/backend_vulkan.c` + `c/backend_vulkan.h` exporting the existing
  `coli_cuda_*` ABI (`init`, `malloc`, `free`, `copy`, `matmul`, `cleanup`).
- Add `VULKAN=1` Makefile path that links `-lvulkan` and keeps CUDA/ROCm builds
  unchanged.
- Add runtime accelerator selection for `COLI_ACCEL=vulkan` without changing CPU,
  CUDA, or ROCm behavior.

**Exit criteria:** `make -C c glm VULKAN=1` succeeds on Mesa/RADV hosts; CPU-only
build still passes `make -C c check`.

#### V1.2 — Shader pipeline and memory model
- Implement compute pipeline creation for q8/q4/q2/f32 kernels with offline
  SPIR-V blobs embedded in C.
- Implement `VkBuffer` allocation/binding, descriptor sets, command recording,
  and queue submission for batched matmul execution.
- Add explicit host-device synchronization and error propagation matching the
  CUDA/ROCm fallback semantics.

**Exit criteria:** Vulkan backend runs quantized matmul correctness parity tests
against the CPU reference on at least one RADV device.

#### V1.3 — Runtime integration and validation
- Add accelerator discovery output for Vulkan devices in planner/doctor surfaces.
- Add Vulkan backend tests (`tests/test_backend_vulkan.c`) with q8/q4/q2/f32
  coverage and CPU fallback assertions.
- Document Fedora package/runtime requirements and troubleshooting for Mesa/RADV.

**Exit criteria:** Vulkan backend path is selectable from CLI, tested in CI
where Vulkan is available, and documented as a supported fallback when ROCm is
not present.

---

## Phase N1 — NPU / XDNA 2 (execution plan)

Goal: offload subgraphs to the AMD **XDNA 2** NPU on Strix Halo
(`/dev/accel/accel0`, visible as `npu` in `coli plan --json`).

The NPU uses a tile-based ISA (AIE2 = AI Engine 2nd gen) and is programmed via:
- **Ryzen AI / IREE**: ONNX → tile program via XRT or Peano compiler
- **ONNX Runtime EP**: `VitisAI` execution provider in ONNX Runtime ≥ 1.19

Candidate workloads for NPU offload:
- MLA attention (fixed-shape per token — NPU likes fixed shapes)
- Shared-expert MLP (always active, same shape every layer)
- Embedding lookup (N/A — memory-bound; NPU doesn't help here)

Key constraints:
- Routed experts must remain on CPU/GPU (variable topk per token → variable
  shape; NPU requires AOT-compiled fixed-shape kernels)
- Synchronisation between NPU, RDNA 4 GPU, and CPU must avoid round-trips
  through the host; use XRT managed memory shared between all three agents.

**Required toolchain (Fedora 43):**
```bash
# AMD XRT (Xilinx Runtime) for XDNA 2
sudo dnf install xrt xrt-devel
# or via amdgpu-install:
amdgpu-install --usecase=aiml
```

**Target interface:**
- `backend_npu.c` / `backend_npu.h`
- `coli_npu_init()`, `coli_npu_matmul()` (same error-fallback pattern)
- `COLI_ACCEL=npu`, detected via `/sys/class/accel/accel0`

### N1 phased execution (Ryzen AI / XDNA 2)

#### N1.1 — Device discovery and capability gates
- Add NPU probe path (`/sys/class/accel/accel*`) with vendor/device filtering for
  AMD XDNA 2.
- Add planner/doctor reporting for NPU availability, driver/runtime versions, and
  unsupported-shape diagnostics.
- Keep hard fallback to CPU/GPU when NPU runtime or shape contracts are not met.

**Exit criteria:** `coli plan --json` and `coli doctor --json` report NPU
availability and clear fallback reasons on non-XDNA systems.

#### N1.2 — Fixed-shape kernel offload path
- Add `backend_npu.c/.h` abstraction and runtime-managed buffers for NPU launch.
- Offload fixed-shape subgraphs first (MLA attention projection/shared expert MLP)
  behind runtime feature flags.
- Add synchronization boundaries between NPU and GPU/CPU execution stages to
  preserve current numerical behavior.

**Exit criteria:** selected fixed-shape workloads execute on XDNA 2 with token
output parity versus CPU baseline.

#### N1.3 — Productionization and observability
- Add per-step telemetry for NPU dispatch count, fallback count, and latency.
- Add integration tests/mocks for NPU unavailable/degraded modes.
- Document Fedora/XRT setup and operational constraints for Ryzen AI systems.

**Exit criteria:** NPU path is observable, resilient, and safe to enable
experimentally via `COLI_ACCEL=npu`.

---

## Phase Q1 — ROCmFPX quantization (execution plan)

Goal: replace per-row int4/int8 with **FP8 (e4m3fnuz)** for GPU-resident
tensors on RDNA 4 / CDNA 3+, where native hardware FP8 arithmetic is available.

| axis | int4 (current) | FP8 e4m3 |
|---|---|---|
| bits/param | 4 | 8 |
| hardware | software dequant in shader | native FP8 multiply on RDNA 4 |
| quality | asymmetric round-to-nearest | closer to BF16; better tail representation |
| dense resident | ~9.9 GB | ~19.8 GB (still fits in 128 GB Strix Halo) |
| disk/expert | keep int4 (bandwidth-limited) | unchanged — FP8 for GPU-resident only |

**Implementation sketch:**
- Add `fmt=4` (FP8 e4m3fnuz) to the `QT` struct in `glm.c`
- `backend_rocm.hip`: use `__hip_fp8_e4m3_fnuz` (ROCm ≥ 6.1) in `weight_at`
  device function; let RDNA 4 use `v_pk_fma_f8` ISA instructions
- Converter (`tools/convert_fp8_to_int4.py`): add `--gpu-bits fp8` option that
  stores GPU-resident tensors in FP8 and disk-streamed experts in int4
- Backward compatible: CPU/CUDA/Vulkan paths keep int4/int8; FP8 activates only
  when the binary is built with `ROCM=1` and the model was converted with
  `--gpu-bits fp8`

### Q1 phased execution (ROCmFPX + rocmfx follow-up)

#### Q1.1 — FP8 resident-tensor format enablement
- Introduce FP8 e4m3fnuz quant format for GPU-resident tensors only, with on-disk
  routed experts remaining int4.
- Add converter mode (`--gpu-bits fp8`) plus metadata tags so runtime can enforce
  ROCm-only activation and safe fallback.
- Preserve backward compatibility for existing int4/int8 model directories.

**Exit criteria:** converter emits mixed-format artifacts, and runtime correctly
accepts/rejects FP8 activation based on backend/device capability.

#### Q1.2 — Native ROCm FP8 arithmetic path on RDNA 4
- Add ROCm kernel path using native FP8 arithmetic types/intrinsics for resident
  matmuls, replacing software int4 dequant on supported devices.
- Keep current int4 path as fallback for unsupported ROCm versions or GPUs.
- Validate quality/perplexity deltas versus current int4 resident path.

**Exit criteria:** RDNA 4 executes native FP8 resident matmuls with expected
quality uplift and memory growth (~2× for resident tensors) while staying within
128 GB unified memory targets.

#### Q1.3 — Extended low-bit roadmap with rocmfx
- Evaluate `rocmfx` (Charlie1234) FP6/FP4 kernels as optional follow-on formats
  for memory/quality trade-off tiers beyond FP8.
- Define compatibility matrix (RDNA generation, ROCm version, kernel availability)
  and integration points with current quant container metadata.
- Gate any FP6/FP4 adoption behind explicit experimental flags until correctness
  and stability match existing int4/FP8 paths.

**Exit criteria:** decision record produced for FP6/FP4 adoption scope, with a
clear experimental path that does not regress existing int4/FP8 users.

---

## Phase F1 — Fedora 43 packaging notes

ROCm is not in the official Fedora repos. Tested install paths:

```bash
# Option A: AMD amdgpu-install (official, deb/rpm)
curl -O https://repo.radeon.com/amdgpu-install/6.3/el/9/amdgpu-install-6.3.60300-1.el9.noarch.rpm
sudo dnf localinstall amdgpu-install-*.rpm
sudo amdgpu-install --usecase=rocm

# Option B: Fedora COPR community repo
sudo dnf copr enable tpitera/rocm
sudo dnf install rocm-hip-sdk hipcc rocm-smi

# Either way, after install:
sudo usermod -aG video,render $(whoami)
# re-login, then:
rocm-smi                          # verify GPU is visible
hipcc --version                   # verify compiler
```

**Strix Halo BIOS:** increase the GPU VRAM carve-out from 512 MB to 4–8 GB in
BIOS → Advanced → AMD CBS → NBIO → GFX Configuration → UMA Frame Buffer Size.
This is only for legacy compatibility; ROCm unified memory uses the full system
RAM regardless.

**Kernel parameters (for full HSA unified memory access):**
```
# /etc/kernel/cmdline.d/amdgpu.conf
amdgpu.sg_display=0
```

---

## Non-goals

- CUDA on AMD (HIP is the target, not CUDA via hipCUDA translation)
- 32-bit Linux: not supported
- Windows ROCm: ROCm on Windows is experimental; track separately if needed
- NPU on non-AMD hardware (Intel NPU, Qualcomm HTP): out of scope for this plan
