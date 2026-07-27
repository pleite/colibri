# NPU Runtime Completion — What's Needed to Finish VNNI Tests

**Date:** 2026-07-26  
**Status:** DRAFT — ready for Copilot implementation  
**Target:** Make the NPU backend actually execute matmuls on XDNA 2 hardware

---

## Current State

The NPU kernel build is complete. 10/15 .npukernel artifacts have been built, committed (commit 7ee6bd8), and pushed. The runtime loads them successfully:

xdna2: loaded kernel npu/kernels/matmul_int8_256x4096x1024.npukernel shape=(256,4096,1024) fmt=1 opcode=20 instr=4208 bytes

However, execution fails at the submit step:

xdna2: ioctl 0xc0186445 failed: Invalid argument

The test output shows all 10 kernels load but none can execute. The NPU backend correctly reports SKIP for these shapes because the DRM dispatch path is incomplete.

---

## Two Known Gaps (from vnni-int8-matmul/npu/aie/README.md)

### Gap 1: Partition Configuration (PDI Registration)

**The problem:** The DRM dispatch path in c/npu_kernels/xdna2_driver.c creates a hardware context via DRM_IOCTL_AMDXDNA_CREATE_HWCTX but **never registers the compiled xclbin's PDI (Partition Description Interface) with the firmware**.

The xclbin is built by mlir-aie + Peano and copied next to each .npukernel artifact (matmul_int8_256x4096x1024.xclbin). The runtime needs to:

1. Load the xclbin file
2. Parse its IP_LAYOUT section to extract compute unit (CU) configuration
3. Register this partition with the firmware via the appropriate DRM ioctl
4. Pass the partition geometry (num_tiles, mem_size, max_opc) to CREATE_HWCTX

**Current code in xdna2_driver.c:**
- xdna2_create_hwctx() creates a hwctx with hardcoded num_tiles=16, mem_size=32768, max_opc=4
- No function exists to load/parse xclbin files
- No function exists to register PDI with firmware

**What needs to be added:**
- A function to load .xclbin files (binary format with ELF-like sections)
- Parse the IP_LAYOUT section to extract CU count, CU mask, memory banks
- A function to register the partition with firmware (likely DRM_IOCTL_AMDXDNA_CREATE_HWCTX with partition info, or a separate DRM_IOCTL_AMDXDNA_LOAD_PDI ioctl)
- Update xdna2_create_hwctx() to accept partition parameters instead of hardcoded values

**Reference:** The xclbin format is documented in XRT's xclbin.h. The xclbinutil tool (already in the AIE toolchain container) can extract IP_LAYOUT information. The kernel driver source at drivers/accel/amdxdna/ has the PDI registration logic.

### Gap 2: ERT Payload Layout

**The problem:** The command payload written by xdna2_matmul.c was written against the documented field order, not against XRT's ert.h structures. It also truncates buffer addresses to 32 bits.

**Current payload (8 words):**
d[0] = instr_addr_lo;   // instruction stream address (truncated to 32 bits)
d[1] = instr_addr_hi;
d[2] = instr_words;     // instruction stream word count
d[3] = x_addr_lo;       // input buffer address (truncated to 32 bits)
d[4] = w_addr_lo;       // weight buffer address (truncated to 32 bits)
d[5] = y_addr_lo;       // output buffer address (truncated to 32 bits)
d[6] = I;               // inner dimension
d[7] = O;               // output dimension

**What needs to be fixed:**
- Rebuild the payload against the actual ert.h structures from XRT's xrt/detail/ert.h
- Use 64-bit addresses (the ert.h structures support this)
- Verify the field order matches what the firmware expects
- The cu_mask is already correct (read from xclbin's IP_LAYOUT)

**Reference:** vnni-int8-matmul/npu/aie/ert_opcode_probe.c reads ERT opcodes from XRT's header. The same header defines the ERT packet structure. The pack_npukernel.py tool already reads the CU mask from the xclbin via xclbinutil.

---

## What's Missing from the Toolchain

### Already Present (in ghcr.io/pleite/colibri-aie-toolchain:latest)

- Peano (llvm-aie) — AIE2/AIE2P code generator
- mlir-aie 1.3.4 — IRON Python API + aiecc
- XRT 2.19.0 — xclbinutil, ert.h, libxrt
- ert-opcode-probe — ERT opcode extraction tool
- pack_npukernel.py — Packs .npukernel artifacts
- build_shape.py — Plans tilings and compiles designs
- build_all.py — Builds all 15 shapes

### NOT Present (but needed for runtime)

- xclbin loader/parser — No existing library loads xclbin files in C. Need to write one or use XRT's xclbin.h (which is C++, not C).
- PDI registration ioctl — The specific ioctl number and structure for registering PDI with firmware is not in the current kernel headers. Need to check drivers/accel/amdxdna/ source or XRT's xrt.h.
- 64-bit address handling — The current payload truncates to 32 bits. Need to verify the firmware supports 64-bit addresses and adjust accordingly.

### Potential Additional Missing Pieces

- XRT's xclbin.h — If we need to parse xclbin files, XRT provides a C++ header. We'd need to either:
  - Write a C parser for the xclbin binary format (ELF-like with custom sections)
  - Use XRT's C++ library (requires linking against libxrt)
  - Parse the xclbin manually using xclbinutil --dump-section IP_LAYOUT and parse the output

- Firmware PDI format — The exact format of the PDI that the firmware expects is not documented in the public kernel headers. It may be:
  - An ELF binary with specific sections
  - A custom binary format
  - Derived from the xclbin's IP_LAYOUT section

---

## Implementation Plan

### Task 1: Research PDI Registration

**Goal:** Understand how the amdxdna driver expects PDI to be registered.

**Steps:**
1. Read drivers/accel/amdxdna/aie2_pdi.c (or similar) in the kernel source
2. Identify the DRM ioctl for PDI registration (if it exists)
3. Determine the PDI format (ELF? custom binary?)
4. Check if CREATE_HWCTX accepts partition info or if a separate ioctl is needed

**Deliverables:**
- Documented PDI registration flow
- ioctl number and structure definition (if applicable)
- PDI format specification

### Task 2: Write xclbin Parser

**Goal:** Parse xclbin files to extract IP_LAYOUT and other sections.

**Options:**
1. Use xclbinutil — Shell out to xclbinutil --dump-section IP_LAYOUT:<file>:binary and parse the binary output
2. Write C parser — Parse the xclbin ELF-like format directly in C
3. Use XRT's C++ library — Link against libxrt_coreutil (requires C++ interop)

**Recommendation:** Option 1 (shell out to xclbinutil) is simplest and avoids maintaining a parser. The xclbinutil output is binary, so we'd need to parse the ELF-like structure.

**Deliverables:**
- Function to load xclbin and extract CU mask, num_tiles, mem_size
- Function to extract PDI binary (if separate from xclbin)

### Task 3: Implement PDI Registration

**Goal:** Register the compiled xclbin's partition with the firmware.

**Steps:**
1. Based on Task 1 research, implement the PDI registration
2. Update xdna2_create_hwctx() to accept partition parameters
3. Load the xclbin next to each .npukernel artifact
4. Pass partition info to CREATE_HWCTX (or call separate PDI ioctl)

**Deliverables:**
- xdna2_load_xclbin() function
- xdna2_register_pdi() function (if separate ioctl)
- Updated xdna2_create_hwctx() signature

### Task 4: Fix ERT Payload Layout

**Goal:** Rebuild the ERT packet against the correct ert.h structures.

**Steps:**
1. Read XRT's xrt/detail/ert.h (already in the AIE toolchain container)
2. Compare field order with current implementation
3. Fix address truncation (use 64-bit addresses if supported)
4. Verify payload size and word count

**Deliverables:**
- Updated ert_packet_t structure in xdna2_matmul.c
- Fixed payload construction
- Verification that payload matches ert.h

### Task 5: Update Runtime to Use Partition Info

**Goal:** Make the runtime use the partition geometry from the xclbin.

**Steps:**
1. Load xclbin when loading kernel
2. Extract num_tiles, mem_size, max_opc from IP_LAYOUT
3. Pass to xdna2_create_hwctx() instead of hardcoded values
4. Update xdna2_matmul.c to use 64-bit addresses

**Deliverables:**
- xdna2_kernel_t struct extended with partition info
- Updated kernel loading flow
- 64-bit address support in ERT payload

### Task 6: Test on Hardware

**Goal:** Verify kernels execute on XDNA 2 NPU.

**Steps:**
1. Run ./scripts/strix-halo-podman-test.sh with kernels
2. Check that submit no longer fails with Invalid argument
3. Verify output correctness (compare against CPU reference)
4. Profile performance

**Deliverables:**
- Test output showing successful NPU execution
- Performance numbers (tok/s, latency)
- Correctness verification

---

## Success Criteria

1. xdna2_submit_command() no longer fails with Invalid argument
2. NPU backend executes at least one matmul successfully (output matches CPU reference)
3. All 10 built kernels load and execute (rows=256 and rows=32 shapes)
4. CI workflow passes with require_npu=true and NPU kernels present
5. No CPU fallback in NPU path (guardrail #1 from docs/strix-halo-npu.md)

---

## References

- c/npu_kernels/xdna2_driver.c — Current DRM ioctl wrapper
- c/npu_kernels/xdna2_matmul.c — Current ERT payload construction
- vnni-int8-matmul/npu/aie/README.md — Known gaps documentation
- docs/strix-halo-npu.md — Full NPU implementation docs
- vnni-int8-matmul/npu/aie/build_all.py — Kernel build script
- vnni-int8-matmul/npu/aie/pack_npukernel.py — Kernel packer
- vnni-int8-matmul/npu/aie/ert_opcode_probe.c — ERT opcode extraction

---

## Notes for Copilot

- The xclbin files are already built and committed next to each .npukernel artifact
- The AIE toolchain container (ghcr.io/pleite/colibri-aie-toolchain:latest) has xclbinutil and XRT headers
- The kernel driver source is at drivers/accel/amdxdna/ in the Linux kernel tree
- XRT 2.19.0 is installed at /opt/xrt/ on Strix Halo
- The ert.h header is at /usr/include/xrt/detail/ert.h (from libxrt-dev)
- Do NOT modify the kernel build scripts — the kernels are already built correctly
- Focus on the runtime: loading xclbin, registering PDI, fixing ERT payload
