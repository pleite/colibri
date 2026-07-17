# Toolchain Setup — Strix Halo (Fedora 43)

**Date:** 2026-07-17
**Hardware:** AMD Ryzen AI Max+ 395 (Strix Halo) — RDNA 4 iGPU (gfx1151) + XDNA 2 NPU
**OS:** Fedora 43, kernel 7.0.12-101.fc43.x86_64

---

## Hardware

| Component | PCI Address | Device ID | Status |
|-----------|------------|-----------|--------|
| RDNA 4 iGPU (Radeon 8060S) | c2:00.0 | 1002:1586 (rev c1) | Active, /dev/dri/card1, /dev/dri/renderD128 |
| XDNA 2 NPU | c3:00.1 | 1022:17f0 (rev 11) | Active, kernel driver amdxdna loaded |

---

## Toolchain 1: Vulkan SDK

**Purpose:** Compile SPIR-V compute shaders for the Vulkan backend (backend_vulkan.c)

**Packages installed (Fedora 43):**

    sudo dnf install -y vulkan-tools vulkan-loader-devel vulkan-validation-layers \
      spirv-tools spirv-tools-devel spirv-headers-devel \
      glslang glslang-devel glslc vulkan-headers

**Key binaries:**

| Tool | Purpose |
|------|---------|
| vulkaninfo | Query Vulkan instance/device capabilities |
| glslc | Compile GLSL to SPIR-V (via glslang + spirv-tools) |
| spirv-as | Assemble SPIR-V text to binary |
| spirv-dis | Disassemble SPIR-V binary to text |
| spirv-val | Validate SPIR-V modules |

**Versions:**
- Vulkan loader: 1.4.341
- glslang: 16.2.0 (shaderc v2026.1)
- spirv-tools: 2026.1
- mesa-vulkan-drivers: 25.3.6 (RADV for RDNA 4)

**Usage for colibri:**

    # Compile a compute shader for quant_matmul
    glslc -O3 --target-env=vulkan1.3 shader.qmatmul.comp -o shader.qmatmul.spv

    # Validate
    spirv-val shader.qmatmul.spv

    # Inspect
    spirv-dis shader.qmatmul.spv

---

## Toolchain 2: ROCm (HIP)

**Purpose:** Compile and run HIP kernels for the ROCm backend (backend_rocm.hip)

**Packages installed (Fedora 43):**

    sudo dnf install -y rocm-devel rocm-core hipcc

**Key binaries:**

| Tool | Purpose |
|------|---------|
| hipcc | HIP compiler driver (clang-based) |
| rocminfo | Query ROCm devices and capabilities |
| rocm-smi | Monitor GPU utilization, memory, power |

**Versions:**
- HIP: 6.4.4 (3484-9999)
- ROCm: 6.4.4
- Clang: 19.0.0git

**Usage for colibri:**

    # Compile HIP kernel
    hipcc -O3 -std=c++17 -o backend_rocm backend_rocm.hip

    # Run
    ./backend_rocm

    # Monitor
    rocm-smi --showusedbandwidth

**Device detected:**

    Name: gfx1151
    Marketing Name: AMD Radeon 8060S Graphics
    Vendor: AMD

---

## Toolchain 3: XRT (XDNA 2 NPU)

**Purpose:** Load XCLBIN kernels and manage XDNA 2 NPU device

**Installation:**
XRT is not in Fedora repos. It was extracted from the colibri-all container:

    podman cp colibri-toolbox:/usr/xrt /opt/xrt
    sudo tee /etc/profile.d/xrt.sh << EOF
    export XILINX_XRT=/opt/xrt
    export LD_LIBRARY_PATH=/opt/xrt/lib64:$LD_LIBRARY_PATH
    export PATH=/opt/xrt/bin:/opt/xrt/bin/unwrapped:$PATH
    EOF
    source /etc/profile.d/xrt.sh

**Key binaries:**

| Tool | Purpose |
|------|---------|
| xclbinutil | Inspect/merge XCLBIN container files |
| xrt-smi | XRT system management (device info, health) |
| xbutil | Xilinx utility (legacy, wraps xrt-smi) |

**Versions:**
- XRT: 2.19.0

**XDNA 2 kernel driver:**
- Module: amdxdna (loaded, version from kernel 7.0.12)
- PCI: c3:00.1 Signal processing controller: AMD Strix/Krackan/Strix Halo Neural Processing Unit
- Vendor: 1022:17f0
- Region 0: 1M MMIO, Region 2: 512K prefetchable (AIE memory)

**Usage for colibri:**

    # Inspect XCLBIN container
    xclbinutil --info --input kernel.xclbin

    # Device info
    xrt-smi info

**Note on AIE compile tools:**
The AIE compiler (aiecompile, aiecompiler) is part of the AMD AIE Tools package, which is separate from XRT and not available in Fedora repos. For XDNA 2 on Strix Halo, the kernel driver handles most runtime operations. Kernel compilation requires:
1. AMD AIE Tools (proprietary, from AMD developer portal)
2. Or the open-source aie-compiler from the Xilinx GitHub
3. These are needed to produce .xclbin files from AIE C/C++ source

For development, the NPU backend can use the CPU fallback path (already implemented) while waiting for actual XRT kernel binaries.

---

## Summary

| Toolchain | Status | Purpose | Backend |
|-----------|--------|---------|---------|
| Vulkan SDK | Installed | SPIR-V shader compilation | backend_vulkan.c |
| ROCm/HIP | Installed | HIP kernel compilation | backend_rocm.hip |
| XRT 2.19 | Installed | XCLBIN loading, NPU management | backend_npu.c |
| AIE Compiler | Not available | XDNA 2 kernel compilation | Future - needs AMD AIE Tools |

The only missing piece is the AIE compiler for producing actual XDNA 2 kernels. The NPU backend framework integration path (PR #50) is complete and ready to dispatch to an XCLBIN when one is available.
