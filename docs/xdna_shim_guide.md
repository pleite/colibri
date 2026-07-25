# XDNA Shim Library Guide for Copilot

## Source
https://rebel7.no/ai/amd-npu-fedora/

## Summary
Complete guide for building and integrating the XDNA shim library that teaches XRT how to drive /dev/accel/accel0.

---

## Status in this repository (validated)

The guide below is upstream material. Only the part of it that can be verified
against hardware has been implemented:

* **Implemented** — `c/npu_kernels/xdna2_xrt_driver.{c,h}`: an XRT availability
  probe built against the official `<xrt/xrt_device.h>` (only when the build
  finds XRT and defines `COLI_NPU_XRT_AVAILABLE`), plus the `XDNA2_DRIVER=auto|drm|xrt`
  policy consumed by `xdna2_runtime_init()`. `XDNA2_DRIVER=xrt` fails with
  `-ENOSYS` when XRT or the shim is missing.
* **Not implemented, deliberately** — an XRT dispatch path
  (`xrtKernelOpen` / `xrtRunStart` / `xrtRunWait`). It needs `.xclbin`
  artifacts and a kernel argument convention from the AIE toolchain; this
  repository only has the `.npukernel` container that the DRM path consumes.
  Writing submit/wait against artifacts that do not exist would be a guessed
  ABI, which the NPU guardrails forbid
  (`docs/strix-halo-npu.md` §6). Add it *with* the artifacts, not before.
* **Unchanged** — the DRM ioctl path remains the only dataflow, and the
  Strix Halo exclusivity and the no-CPU-fallback rule are untouched. The
  `vnni-int8-matmul` tree links the probe but never links XRT.

The "Files to Create/Modify" and "Testing Strategy" sections below therefore
describe the *remaining* work, not the current state.

---

## What is the XDNA Shim?

The XDNA shim is a **SHIM library** that bridges XRT (Xilinx Runtime) to the /dev/accel/accel0 device. XRT itself doesn't know how to drive the NPU device - it needs this shim library.

### Key Components
- **libxrt_driver_xdna.so.2.23.0** - The actual SHIM library
- **Location**: /opt/xilinx/xrt/lib64/ (after installation)
- **Purpose**: Teaches XRT how to communicate with /dev/accel/accel0

---

## Build Process (Phase 3 from Guide)

### Prerequisites
- XRT already built and installed at /opt/xilinx/xrt/
- Kernel 7.0+ with in-tree amdxdna driver (we have 7.1.3)
- Firmware version >= 1.1.0.0 (we have 1.1.2.65)

### Build Steps

```bash
# 1. Clone xdna-driver repository (if not already done)
git clone --recursive https://github.com/amd/xdna-driver.git
cd xdna-driver

# 2. Navigate to plugin build directory
cd build

# 3. Apply cmake4 patches (same as XRT build)
# Replace cmake3 fallback block with no-op

# 4. Set environment variables
export CMAKE_POLICY_VERSION_MINIMUM=3.5
export CPATH=~/local-include:$CPATH  # For ocl_icd.h if needed
source /opt/xilinx/xrt/setup.sh

# 5. Build the plugin
./build.sh -release -nokmod
# -release: Release build
# -nokmod: Skip kernel module (we use in-tree driver)

# 6. Build produces:
#    Release/src/shim/libxrt_driver_xdna.so.2.23.0
#    Release/xrt_plugin.2.23.0_44-x86_64-amdxdna.rpm
```

### Installation

```bash
# Install with --nodeps (avoid dependency conflicts)
sudo rpm -ivh --nodeps ./Release/xrt_plugin.2.23.0_44-x86_64-amdxdna.rpm

# This installs:
# - libxrt_driver_xdna.so to /opt/xilinx/xrt/lib64/
# - VTD archives to /opt/xilinx/xrt/share/amdxdna/bins/
```

### Verification

```bash
# Test with xrt-smi
sudo /opt/xilinx/xrt/bin/xrt-smi examine

# Expected output:
# XRT Version: 2.23.0
# amdxdna Version: 7.0.6-200.fc44.x86_64
# NPU Firmware Version: 1.1.2.64
# Device(s) Present:
# |BDF             |Name          |Architecture  |Topology  |
# |[0000:c3:00.1]  |RyzenAI-npu6  |aie2p         |6x8       |
```

---

## Integration Strategy for Our Project

### Current Approach (DRM ioctl)
- Direct kernel interface via /dev/accel/accel0
- Custom xdna2_driver.c using DRM ioctls
- Bypasses XRT entirely

### Recommended Approach (XRT + Shim)
- Use XRT APIs through the shim library
- More official AMD support
- Better integration with ecosystem

### Hybrid Approach (Recommended)
1. **Keep existing xdna2_driver.c** as fallback
2. **Add XRT-based implementation** using shim library
3. **Select implementation at runtime** based on availability
4. **Maintain backward compatibility**

---

## Files to Create/Modify

### 1. New File: c/npu_kernels/xdna2_xrt_driver.c
- XRT-based NPU driver implementation
- Uses libxrt_driver_xdna.so
- Implements same interface as xdna2_driver.c

### 2. New File: c/npu_kernels/xdna2_xrt_driver.h
- Header for XRT-based driver
- Function declarations matching xdna2_driver.h

### 3. Modified: c/npu_kernels/CMakeLists.txt
- Add XRT library paths
- Link against libxrt_core.so, libxrt_coreutil.so
- Link against libxrt_driver_xdna.so

### 4. Modified: c/npu_kernels/xdna2_matmul.c
- Add runtime selection logic
- Try XRT first, fallback to DRM ioctl

### 5. Modified: tests/test_npu.c
- Test both XRT and DRM ioctl paths
- Validate both implementations

---

## XRT API Usage (Based on Guide)

### Key Libraries
- libxrt_core.so - Core XRT functionality
- libxrt_coreutil.so - Utility functions
- libxrt_driver_xdna.so - XDNA shim (our target)

### Key Functions (to be discovered in XRT headers)
```c
// Device enumeration
xrtDeviceHandle xrtDeviceOpen(const char* deviceName);

// Buffer management
xrtBufferHandle xrtBufferAlloc(xrtDeviceHandle device, size_t size);
void* xrtBufferMap(xrtBufferHandle buffer);
void xrtBufferUnmap(xrtBufferHandle buffer);

// Command submission
xrtCommandHandle xrtCommandSubmit(xrtDeviceHandle device, ...);
int xrtCommandWait(xrtCommandHandle command);

// Synchronization
void xrtDeviceSync(xrtDeviceHandle device, ...);
```

### VTD Archives
The shim uses VTD (Vendor Target Description) archives:
- xrt_smi_strx.a - Strix Halo NPU
- xrt_smi_phx.a - Phoenix NPU
- xrt_smi_npu3.a - NPU3

These contain hardware-specific configuration for each NPU variant.

---

## Build Dependencies

### System Packages (Fedora)
```bash
# Already installed in our system
sudo dnf install -y \
  boost-devel boost-filesystem boost-program-options boost-static \
  cmake cppcheck curl dkms dmidecode \
  elfutils-devel elfutils-libs \
  gcc gcc-c++ gdb git \
  glibc-static gnuplot gnutls-devel gtest-devel \
  json-glib-devel libcurl-devel libdrm-devel libffi-devel \
  libjpeg-turbo-devel libpng12-devel libstdc++-static libtiff-devel \
  libuuid-devel libyaml-devel lm_sensors make \
  opencl-headers openssl-devel pciutils perl pkgconf-pkg-config \
  protobuf-compiler protobuf-devel \
  python3 python3-devel python3-pip python3-sphinx pybind11-devel \
  rapidjson-devel rpm-build strace \
  systemd-devel systemtap-sdt-devel unzip \
  kernel-headers ninja-build
```

### Environment Setup
```bash
# Source XRT setup
source /opt/xilinx/xrt/setup.sh

# Verify XRT is accessible
xrt-smi examine
```

---

## Testing Strategy

### Phase 1: Build and Install Shim
1. Clone xdna-driver repository
2. Build XRT (if not already built)
3. Build XDNA plugin/shim
4. Install to /opt/xilinx/xrt/
5. Verify with xrt-smi examine

### Phase 2: Implement XRT Driver
1. Create xdna2_xrt_driver.c
2. Implement device open/close
3. Implement buffer allocation/deallocation
4. Implement command submission
5. Test with existing test harness

### Phase 3: Integration
1. Add runtime selection to xdna2_matmul.c
2. Test both XRT and DRM ioctl paths
3. Validate performance difference
4. Update CMakeLists.txt

### Phase 4: Validation
1. Run full test suite with XRT
2. Compare results with DRM ioctl
3. Benchmark performance
4. Document findings

---

## Performance Expectations

### XRT vs DRM ioctl
- **XRT**: More overhead due to abstraction layer, but better integration
- **DRM ioctl**: Lower overhead, direct kernel access
- **Expected difference**: 5-15% slower with XRT for simple operations
- **Benefit**: XRT provides better error handling, synchronization, and ecosystem integration

### Use Cases
- **XRT**: Production deployments, official AMD support, ecosystem integration
- **DRM ioctl**: Performance-critical paths, custom optimizations, fallback

---

## Next Steps for Copilot

1. **Clone xdna-driver repository**
2. **Build XRT from source** (if needed)
3. **Build XDNA plugin/shim**
4. **Install to /opt/xilinx/xrt/**
5. **Verify with xrt-smi**
6. **Create xdna2_xrt_driver.c**
7. **Integrate with existing codebase**
8. **Test and validate**

---

## References

- **XRT Repository**: https://github.com/amd/xdna-driver
- **XRT Documentation**: https://docs.xilinx.com/r/en-US/xrt
- **AMD NPU Guide**: https://rebel7.no/ai/amd-npu-fedora/
- **Our XRT Installation**: /opt/xilinx/xrt/
- **Our NPU Device**: /dev/accel/accel0

---

## Summary

The XDNA shim library is the missing piece to properly integrate with XRT. It provides the bridge between XRT APIs and the /dev/accel/accel0 device. Building and integrating this shim will give us official AMD support and better ecosystem integration, while maintaining our existing DRM ioctl implementation as a fallback for performance-critical paths.
