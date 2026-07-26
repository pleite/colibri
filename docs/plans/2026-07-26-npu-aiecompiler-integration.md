# NPU AIE Compiler Integration — Implementation Plan

**Date:** 2026-07-26
**Target:** Strix Halo (Ryzen AI Max+ 395) — XDNA 2 NPU
**Status:** DRAFT — awaiting Copilot implementation

---

## Problem Statement

The colibri NPU backend (c/npu_kernels/) is fully wired for DRM ioctl dispatch
through /dev/accel/accel0. It consumes .npukernel artifacts — self-describing
containers that hold a DPU instruction stream plus an ERT opcode and CU mask.

**The missing piece:** no .npukernel artifacts exist. They must be produced by
AMD s AIE toolchain (aiecompiler) from AIE C/C++ source describing the
fixed-shape INT8 matmul kernels.

Without compiled kernels, every NPU dispatch returns -ENOENT and the NPU
backend is effectively dead code — which is fine for CI (tests report SKIP
correctly) but blocks production inference.

---

## Current State

| Component | Status | Details |
|-----------|--------|---------|
| DRM ioctl driver (xdna2_driver.c) | OK Working | Device open, hwctx, BOs, submit, wait, queries |
| XRT probe (xdna2_xrt_driver.c) | OK Working | Availability probe only, no dispatch path |
| Matmul runtime (xdna2_matmul.c) | OK Working | Kernel loading, DMA, dispatch, sync |
| Shape enumeration (npu_shapes.c) | OK Done | 5 projections x 3 row tiles = 15 artifacts |
| Podman test harness | OK Working | Builds and runs in container with NPU passthrough |
| CI workflow | OK Working | NPU tests fail loudly when device is absent |
| XRT 2.19.0 | OK Installed | At /opt/xrt/ on Strix Halo, from container extraction |
| XDNA shim (libxrt_driver_xdna.so) | MISSING | Needs build from xdna-driver repo |
| AIE compiler (aiecompiler) | MISSING | Separate tool, not in Fedora repos |
| .npukernel artifacts | MISSING | Must be produced by aiecompiler |

---

## What the AIE Compiler Does

aiecompiler takes AIE C/C++ source describing a tiled matmul (rows x inner x
out, INT8 MAC) and produces:

1. DPU instruction stream — the binary that the XDNA 2 firmware executes in
   the AIE-2 array (10 cols x 8 rows, partitionable)
2. ERT opcode — the packet format the driver expects
3. CU mask — which compute units are active

These three pieces go into the .npukernel container the runtime already
consumes. The container format is defined in docs/strix-halo-npu.md section 3.

---

## The 15 Kernels to Compile

From vnni-int8-matmul/sched/npu_shapes.c:

| Projection | Inner | Out | Role |
|------------|-------|-----|------|
| expert gate/up | 4096 | 1024 | Expert FFN gate/up projections |
| expert down | 1024 | 4096 | Expert FFN down projection |
| q_proj | 4096 | 16384 | Self-attention query projection |
| k/v_proj | 4096 | 512 | Self-attention key/value projection |
| o_proj | 8192 | 4096 | Self-attention output projection |

Crossed with row tiles: 256 (large prefill), 32 (small prefill), 1 (decode).

That is 5 x 3 = 15 .npukernel artifacts.

---

## Plan for Copilot

### Task 1: Set up the AIE compiler environment

**Goal:** Get aiecompiler runnable on Strix Halo or inside the podman test
container.

**Approach options (ranked by feasibility):**

1. AMD AIE Tools (proprietary) — Download from AMD developer portal. This is
   the canonical path. Requires AMD account, downloads a tarball with
   aiecompiler, aiecompiler-rt, and AIE runtime libraries.

2. Open-source aie-compiler — The Xilinx GitHub has an open-source AIE
   compiler (xilinx/aiecompiler) that supports AIE-2. Check if it covers
   XDNA 2 (Strix Halo uses XDNA 2, which is a derivative of AIE-2).

3. Pre-compiled artifacts — If AMD has published reference kernels for
   Strix Halo, download those directly and skip the compiler entirely.

**Deliverables:**
- aiecompiler binary accessible in PATH or via a wrapper script
- AIE runtime libraries in the library search path
- Verification: aiecompiler --version or equivalent works

### Task 2: Write the AIE C source for the matmul kernel

**Goal:** AIE C/C++ source that describes a fixed-shape INT8 matmul.

**Requirements:**
- Tiled matmul: A (rows x inner), B (inner x out) -> C (rows x out)
- INT8 MAC datapath (AIE-2 has no INT4 MAC)
- Partitionable across the 10x8 AIE array
- Must produce a kernel that the XDNA 2 firmware can execute
- Must match the ERT packet format the DRM driver expects

**File:** c/npu_kernels/aie/src/matmul_int8.aie (or similar)

**Note:** The exact AIE C syntax and tile partitioning depends on the AIE
toolchain version and XDNA 2-specific constraints. The Copilot worker needs
to research the AIE C API for AIE-2/XDNA 2.

### Task 3: Build all 15 kernel artifacts

**Goal:** Compile each (rows, inner, out) combination into a .npukernel file.

**Script:** scripts/build-npu-kernels.sh

**Steps:**
1. For each of the 15 shapes from npu_shapes.c:
   a. Invoke aiecompiler with the AIE C source and shape parameters
   b. Extract DPU instruction stream, ERT opcode, CU mask
   c. Pack into .npukernel container format
   d. Place in npu/kernels/ directory

**Deliverables:** 15 .npukernel files in npu/kernels/

### Task 4: Integrate into the build system

**Goal:** Make kernel compilation part of the normal build.

**Changes:**
- Add make npu-kernels target to vnni-int8-matmul/Makefile
- Add COLI_NPU_KERNEL_DIR env var support (already defined in runtime)
- Update Dockerfile.strix-halo-test to include aiecompiler if needed
- Update podman test harness to build kernels before running tests

### Task 5: Add kernel validation to CI

**Goal:** CI verifies kernels are present and the NPU backend loads them.

**Changes to .github/workflows/vnni-test.yml:**
- Add step to verify .npukernel files exist before running tests
- Add step to verify NPU backend loads at least one kernel
- Update the assert no backend silently fell back check to require NPU
  kernels when require_npu=true

### Task 6: (Optional) Build the XDNA shim

**Goal:** Get libxrt_driver_xdna.so working for XRT-based dispatch.

**This is lower priority** because the DRM ioctl path is fully functional and
the .npukernel format doesn't use XRT s .xclbin/xrtKernelOpen API. The
XRT probe (xdna2_xrt_driver.c) only validates XRT availability — it doesn't
dispatch work.

**Only do this if:**
- AMD publishes XRT-compatible kernel artifacts for XDNA 2
- There is a performance or feature advantage to using XRT over DRM ioctls

---

## Integration with Podman Container

The existing podman test harness (scripts/strix-halo-podman-test.sh) already
passes /dev/accel/accel0 into the container. The kernel compilation (Task 3)
should happen on the host (Strix Halo), not inside the container, because:

1. The AIE compiler is large and slow to build
2. Kernel artifacts are immutable for a given kernel version
3. The container should only test the runtime, not the toolchain

**Recommended approach:**
1. Build kernels on the host -> place in npu/kernels/
2. The podman harness already mounts the repo root, so kernels are visible
3. CI builds kernels as a separate step before make podman-test

---

## Integration with Copilot Workflow

Copilot workers have been used extensively in this repo. The prompt should:

1. Reference the existing codebase — c/npu_kernels/, vnni-int8-matmul/
2. Cite the guardrails — docs/strix-halo-npu.md section 6 (no CPU fallback, no
   guessed ERT opcodes, no hand-copied UAPI)
3. Provide the shape set — vnni-int8-matmul/sched/npu_shapes.c
4. Specify the container format — the .npukernel binary layout in section 3 of
   docs/strix-halo-npu.md
5. Require hardware validation — kernels must be tested on actual XDNA 2
   hardware, not just compile cleanly

---

## Dependencies

| Dependency | Status | How to get |
|------------|--------|------------|
| aiecompiler binary | MISSING | AMD developer portal or open-source AIE repo |
| AIE runtime libraries | MISSING | Bundled with aiecompiler |
| <drm/amdxdna_accel.h> | OK | kernel-headers >= 6.14 (installed) |
| XRT 2.19.0 | OK | /opt/xrt/ on Strix Halo |
| Linux 7.0+ with amdxdna | OK | 7.0.12-101.fc43.x86_64 |
| NPU firmware | OK | /lib/firmware/amdnpu/17f0_11/ |
| Podman | OK | Installed on Strix Halo |

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| aiecompiler not available for XDNA 2 | Blocks all NPU work | Use pre-compiled reference kernels; fall back to CPU-only CI |
| AIE C syntax differs from docs | Wasted Copilot effort | Provide exact toolchain version in prompt |
| Kernel artifacts too large for repo | CI/CD bloat | Store artifacts in GitHub releases or artifact storage |
| XDNA 2 firmware doesn't support all shapes | Partial NPU coverage | Document which shapes work; gate CI on actual hardware results |
| AIE compiler is proprietary/locked | Cannot distribute | Build on CI runner, cache artifacts, don't commit binaries |

---

## Success Criteria

1. aiecompiler runs on Strix Halo (bare metal or container)
2. All 15 .npukernel artifacts are produced and placed in npu/kernels/
3. make podman-test on Strix Halo reports NPU backend OK (not SKIP)
4. CI workflow passes with require_npu=true and NPU kernels present
5. No CPU fallback in NPU path (guardrail #1 from docs/strix-halo-npu.md)

---

## References

- docs/strix-halo-npu.md — Full NPU implementation docs, guardrails, execution model
- docs/xdna_shim_guide.md — XRT/XDNA shim build guide (for future XRT dispatch)
- vnni-int8-matmul/sched/npu_shapes.c — Shape enumeration (the 15 kernels)
- vnni-int8-matmul/scripts/strix-halo-podman-test.sh — Podman harness
- c/npu_kernels/xdna2_matmul.c — Runtime that consumes .npukernel artifacts
- https://github.com/amd/xdna-driver — XRT plugin source (for XDNA shim)
- https://rebel7.no/ai/amd-npu-fedora/ — Community XDNA guide

---

## AIE Compiler Research Findings (2026-07-26)

### What we looked for

The canonical AIE compiler for XDNA 2 / Strix Halo. We searched:

1. **Xilinx/AMD official repos** — , , , , 
2. **Open-source alternatives** — GitHub search for , 
3. **Community projects** — , 

### Results

| Repo | Status | Notes |
|------|--------|-------|
|  | 404 | Does not exist |
|  | 404 | Does not exist |
|  | 404 | Does not exist |
|  | No releases | XRT is distributed via AMD website, not GitHub releases |
|  | v2.21.75 | XRT plugin source, NOT the AIE compiler |
|  | Active (20 stars) | Open-source AIE compiler, targets AIE-ML/AIE-MLv2 (Versal), requires AMD Vitis 2025.2 + license |
|  | Unknown | MLIR-based, Versal-focused |

### Key finding: aie4ml does NOT support XDNA 2

 is the only open-source AIE compiler we found, but:
- It targets **AIE-ML and AIE-MLv2** (Versal FPGAs), not XDNA 2
- It requires **AMD Vitis 2025.2** (proprietary, 4129841298)
- It requires a **valid AIE tools license**
- It is a Python package () that generates AIE projects for Vitis
- It has 20 stars, 1 open issue (contribution question), last updated July 2026

**This is not usable for Strix Halo XDNA 2.** The AIE-2 in Versal and the XDNA 2 in Strix Halo share a lineage but have different firmware interfaces, tile configurations, and instruction sets.

### The Xilinx/AMD official AIE compiler

AMD's official  is:
- **Proprietary** — part of the Vitis AIE Tools suite
- **Not publicly available** — requires AMD developer account and NDA
- **Not in any package manager** — distributed as a tarball download
- **Targeted at Versal/FPGA** — XDNA 2 support may or may not be included
- **Separate from XRT** — XRT handles runtime, aiecompiler handles build

### Recommended path forward

Given that no publicly available aiecompiler supports XDNA 2:

1. **Contact AMD** — Request access to the XDNA 2 AIE toolchain. This is the
   canonical path and what any production Strix Halo NPU deployment would use.

2. **Reverse-engineer from existing firmware** — The Strix Halo NPU firmware
   at  contains the AIE-2 microcode. If AMD
   publishes reference kernels or kernel templates for Strix Halo, we can
   adapt them.

3. **Use pre-compiled reference kernels** — If AMD has published  or
   equivalent artifacts for Strix Halo (e.g., in the XRT package or as a
   separate download), extract the DPU instruction streams and pack them into
   our  format.

4. **Write raw DPU instructions** — The AIE-2 instruction set is documented in
   the Linux kernel driver (). We could write
   the instruction streams by hand for simple matmul kernels. This is tedious
   but does not require any external toolchain.

5. **Defer NPU kernel development** — Keep the DRM ioctl path as a validated
   test harness. Without compiled kernels, the NPU backend correctly reports
   SKIP. Focus on Vulkan and ROCm backends which have working toolchains.

### What Copilot should do

Given the aiecompiler availability situation, the Copilot prompt should:

1. **Assume path 5 (defer)** as the primary path — the NPU backend is already
   a fully validated test harness that reports SKIP correctly
2. **Include path 4 (raw DPU) as a future task** — document the AIE-2 instruction
   set from the kernel headers so a future worker can write kernels by hand
3. **Add a research task** — have Copilot reach out to AMD or check if reference
   kernels are available before spending engineering time on kernel development
4. **Keep CI working** — the podman test harness and CI workflow already work
   correctly with SKIP on the NPU path. Do not break this.

### Immediate next steps

1. Add a research task to check AMD developer portal for XDNA 2 AIE tools
2. Document the AIE-2 instruction set from kernel headers
3. Keep the NPU backend as a validated SKIP path in CI
4. Focus engineering effort on Vulkan (SPIR-V) and ROCm (HIP) backends which
   have working open toolchains

---

## AIE Compiler Research Findings (2026-07-26)

### What we looked for

The canonical AIE compiler for XDNA 2 / Strix Halo. We searched:

1. Xilinx/AMD official repos: Xilinx/aiecompiler, Xilinx/AMD-AIE, amd/aiecompiler, amd/XRT, amd/xdna-driver
2. Open-source alternatives: GitHub search for aiecompiler, aie compiler amd
3. Community projects: dimdano/aie4ml, hanchenye/polyaie

### Results

- Xilinx/aiecompiler: 404 - Does not exist
- Xilinx/AMD-AIE: 404 - Does not exist
- amd/aiecompiler: 404 - Does not exist
- amd/XRT: No releases on GitHub (distributed via AMD website)
- amd/xdna-driver: v2.21.75 - XRT plugin source, NOT the AIE compiler
- dimdano/aie4ml: Active (20 stars) - Open-source AIE compiler, targets AIE-ML/AIE-MLv2 (Versal), requires AMD Vitis 2025.2 + license
- hanchenye/polyaie: MLIR-based, Versal-focused

### Key finding: aie4ml does NOT support XDNA 2

dimdano/aie4ml is the only open-source AIE compiler we found, but:
- It targets AIE-ML and AIE-MLv2 (Versal FPGAs), not XDNA 2
- It requires AMD Vitis 2025.2 (proprietary, expensive)
- It requires a valid AIE tools license
- It is a Python package (pip install aie4ml) that generates AIE projects for Vitis
- It has 20 stars, 1 open issue (contribution question), last updated July 2026

This is not usable for Strix Halo XDNA 2. The AIE-2 in Versal and the XDNA 2 in Strix Halo share a lineage but have different firmware interfaces, tile configurations, and instruction sets.

### The Xilinx/AMD official AIE compiler

AMD s official aiecompiler is:
- Proprietary - part of the Vitis AIE Tools suite
- Not publicly available - requires AMD developer account and NDA
- Not in any package manager - distributed as a tarball download
- Targeted at Versal/FPGA - XDNA 2 support may or may not be included
- Separate from XRT - XRT handles runtime, aiecompiler handles build

### Recommended path forward

Given that no publicly available aiecompiler supports XDNA 2:

1. Contact AMD - Request access to the XDNA 2 AIE toolchain. This is the
   canonical path and what any production Strix Halo NPU deployment would use.

2. Reverse-engineer from existing firmware - The Strix Halo NPU firmware
   at /lib/firmware/amdnpu/17f0_11/ contains the AIE-2 microcode. If AMD
   publishes reference kernels or kernel templates for Strix Halo, we can
   adapt them.

3. Use pre-compiled reference kernels - If AMD has published .xclbin or
   equivalent artifacts for Strix Halo (e.g., in the XRT package or as a
   separate download), extract the DPU instruction streams and pack them into
   our .npukernel format.

4. Write raw DPU instructions - The AIE-2 instruction set is documented in
   the Linux kernel driver (drivers/accel/amdxdna/aie2_fw.h). We could write
   the instruction streams by hand for simple matmul kernels. This is tedious
   but does not require any external toolchain.

5. Defer NPU kernel development - Keep the DRM ioctl path as a validated
   test harness. Without compiled kernels, the NPU backend correctly reports
   SKIP. Focus on Vulkan and ROCm backends which have working toolchains.

### What Copilot should do

Given the aiecompiler availability situation, the Copilot prompt should:

1. Assume path 5 (defer) as the primary path - the NPU backend is already
   a fully validated test harness that reports SKIP correctly
2. Include path 4 (raw DPU) as a future task - document the AIE-2 instruction
   set from the kernel headers so a future worker can write kernels by hand
3. Add a research task - have Copilot reach out to AMD or check if reference
   kernels are available before spending engineering time on kernel development
4. Keep CI working - the podman test harness and CI workflow already work
   correctly with SKIP on the NPU path. Do not break this.

### Immediate next steps

1. Add a research task to check AMD developer portal for XDNA 2 AIE tools
2. Document the AIE-2 instruction set from kernel headers
3. Keep the NPU backend as a validated SKIP path in CI
4. Focus engineering effort on Vulkan (SPIR-V) and ROCm (HIP) backends which
   have working open toolchains
