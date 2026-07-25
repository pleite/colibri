# NPU Kernels — XDNA 2 (Strix Halo)

## Overview

This directory contains the NPU backend implementation for AMD XDNA 2 on Strix Halo.
The NPU provides ~30 TOPS INT8 inference via the AIE-2 (AI Engine) array.

## Hardware

- **NPU**: AMD XDNA 2 (Ryzen AI Max+ 395)
- **AIE Array**: 10 columns × 8 rows (80 AIE cores), partitionable
- **Memory**: 24MB on-chip SRAM (tile memory) + unified 65GB host DRAM
- **Clock**: H-clock up to 1.3 GHz, NPU clock up to 1.1 GHz
- **Performance**: ~30 TOPS INT8, ~15 TOPS INT4
- **Constraint**: Fixed-shape kernels only (AIE-2 tile ISA)

## Architecture

```
Host (Strix Halo CPU)
  backend_npu.c (dispatcher)
    coli_npu_matmul()
    Tensor upload/eviction
      |
  npu_kernels/xdna2_driver.c (DRM ioctl)
    - Hardware context management
    - Buffer object allocation
    - Command submission
      |
  Kernel: amdxdna (Linux DRM driver) via /dev/dri/card1
    - Partition management
    - DMA scheduling
      |
  XDNA 2 NPU Hardware
    - AIE-2 tile array (10x8)
    - Fixed-shape kernel execution
    - Doorbell-based command submission
```

## Files

| File | Lines | Purpose |
|------|-------|---------|
| `xdna2_driver.h` | 210 | DRM ioctl wrapper API, buffer management, command submission |
| `xdna2_driver.c` | 409 | Implementation of DRM ioctl wrapper |
| `xdna2_matmul.h` | 155 | NPU matmul kernel interface |
| `xdna2_matmul.c` | 317 | NPU matmul kernel (CPU fallback, NPU path stub) |
| `npu_runtime.h` | 159 | Runtime integration header (coli_npu_* API) |
| `CMakeLists.txt` | 64 | Build system |
| `tests/test_npu.c` | 296 | Test harness (6 test cases) |

## Compilation

### Direct gcc (no CMake):
```bash
cd /home/leite/colibri
gcc -O3 -march=native -std=c11 -Wall     -I c/npu_kernels     c/npu_kernels/xdna2_driver.c     c/npu_kernels/xdna2_matmul.c     c/npu_kernels/tests/test_npu.c     -o c/npu_kernels/test_npu     -ldl -lpthread -lm
```

### With CMake:
```bash
cd /home/leite/colibri
mkdir -p build-npu && cd build-npu
cmake ../c -DCOLI_NPU=ON
make test_npu
```

## Testing

```bash
# Run tests
./c/npu_kernels/test_npu

# Verify NPU is accessible
ls -la /dev/dri/card1
cat /sys/class/accel/accel0/device/driver/module/version
lsmod | grep amdxdna
```

## Implementation Status

### Complete (ready to compile and test):
- [x] DRM ioctl wrapper (`xdna2_driver.h/c`) — full C API
- [x] Matmul kernel interface (`xdna2_matmul.h/c`) — CPU fallback working
- [x] Runtime integration header (`npu_runtime.h`) — matches backend_npu.c API
- [x] Build system (`CMakeLists.txt`)
- [x] Test harness (`tests/test_npu.c`) — 6 test cases

### Pending (requires aiecompiler):
- [ ] AIE kernel compilation (`xdna2_compile_kernel`)
- [ ] NPU kernel execution path (currently CPU fallback)
- [ ] Pre-compiled kernel binaries for common shapes

### aiecompiler acquisition:
The `aiecompiler` is NOT included in the base XRT install.
It must be downloaded from AMD's XRT GitHub releases:
```
https://github.com/amd/XRT/releases
```
Look for `aiecompiler` in the release assets, or install via:
```bash
pip install aiecompiler  # if available
# or download the standalone binary from AMD XRT releases
```

## NPU vs CPU vs GPU Decision Matrix

| Operation | Shape | Best Backend | Reason |
|-----------|-------|-------------|--------|
| int8 matmul (large) | 4096x4096 | GPU (HIP) | Highest throughput |
| int8 matmul (small) | 1x4096 | NPU or CPU | NPU has latency advantage |
| int4 matmul | variable | CPU | int4 needs dequant first |
| RMSNorm | 4096 | CPU | Too small for NPU/GPU overhead |
| Softmax | 4096 | CPU | Element-wise, CPU is fastest |
| Router (top-k) | 512x60 | CPU | Very small |
| Shared expert | 4096x4096 | NPU | Fixed shape, high TOPS |

## References

- AMD XDNA driver: `/usr/include/drm/amdxdna_accel.h`
- XRT headers: `/opt/xrt/include/`
- XRT libs: `/opt/xrt/lib64/`
- XRT version: 2.19.0 (Apr 2025)
- Kernel module: `amdxdna` (CONFIG_DRM_ACCEL_AMDXDNA=m)
- Firmware: `/lib/firmware/amdnpu/17f0_11/`
