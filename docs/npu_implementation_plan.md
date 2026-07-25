# NPU Implementation Plan for Copilot

## Status

**Committed**: `a106cb1` on `main` — NPU kernel framework files added to `c/npu_kernels/`

**Test Results on Strix Halo** (4/6 passing):
- Device discovery: PASS (AIE 8x6, version 1.1, 48 tiles)
- Buffer objects: PASS (create, map, sync, verify data integrity)
- Matmul CPU fallback: PASS (0% error, shape 1x64x64)
- Shape registration: PASS
- HW context creation: SKIP (partition conflict — needs root or udev rule)
- Runtime lifecycle: SKIP (depends on hwctx)

## Files Already in Place

| File | Lines | Status |
|------|-------|--------|
| `c/npu_kernels/xdna2_driver.h` | 210 | Complete API |
| `c/npu_kernels/xdna2_driver.c` | 409 | DRM ioctl wrapper (compiles, device query works) |
| `c/npu_kernels/xdna2_matmul.h` | 155 | Complete API |
| `c/npu_kernels/xdna2_matmul.c` | 317 | CPU fallback working, NPU path stub |
| `c/npu_kernels/npu_runtime.h` | 159 | coli_npu_* API matching backend_npu.c |
| `c/npu_kernels/CMakeLists.txt` | 64 | Build system |
| `c/npu_kernels/tests/test_npu.c` | 296 | 6 test cases |
| `c/npu_kernels/README.md` | 125 | Architecture docs |

## What Copilot Needs to Implement

### Phase 1: Fix Hardware Context Creation (PRIORITY: HIGH)

The `xdna2_create_hwctx()` function fails with "Operation not supported".
This is likely because:
1. The NPU is already partitioned by the OS (default partition may be active)
2. The ioctl struct layout may not match the kernel's expectations
3. Permissions issue (needs `video` group or root)

**Tasks:**
1. Check if a default NPU partition exists: `cat /sys/class/accel/accel0/device/partition`
2. Try creating hwctx with minimal params (0 tiles, 0 mem)
3. If partition exists, query it instead of creating new
4. Add udev rule for non-root access: `/etc/udev/rules.d/99-amdxdna.rules`

### Phase 2: Implement NPU Matmul Execution Path

The `xdna2_matmul_int8()` function currently falls back to CPU.
The NPU path requires:

1. **Allocate BOs** for input (x), weights, output (y) using `xdna2_create_bo()`
2. **Sync data to device** using `xdna2_sync_bo(fd, bo, SYNC_DIRECT_TO_DEVICE, ...)`
3. **Build exec command** with kernel args (BO handles + offsets)
4. **Submit command** via `xdna2_submit_command()`
5. **Wait for completion** (simplified — non-blocking for now)
6. **Sync results back** using `xdna2_sync_bo(fd, output_bo, SYNC_DIRECT_FROM_DEVICE, ...)`

**The exec command args format:**
```c
// Each arg is a struct with BO handle + offset:
struct exec_buf_arg {
    uint32_t bo_handle;
    uint32_t pad;
    uint64_t offset;
    uint64_t size;
};
```

### Phase 3: AIE Kernel Compilation (REQUIRES aiecompiler)

This phase requires the `aiecompiler` tool which is NOT installed.

**To get aiecompiler:**
```bash
# Option 1: Download from AMD XRT releases
# https://github.com/amd/XRT/releases — look for aiecompiler assets

# Option 2: Install via pip (if available)
pip install aiecompiler

# Option 3: Build from source (complex, requires LLVM/MLIR)
```

**Once aiecompiler is available:**
1. Generate AIE C source for each matmul shape
2. Compile with `aiecompiler-mlir --target=aie --aie-version=2`
3. Package to `.xclbin` format
4. Load via DRM ioctl

**Recommended shapes to pre-compile for Ornith 397B:**
- `matmul_int8(1, 4096, 4096)` — single-row batch, main expert path
- `matmul_int8(1, 4096, 1024)` — small intermediate
- `matmul_int8(1, 1024, 4096)` — projection
- `matmul_int8(8, 4096, 4096)` — batch of 8 tokens

### Phase 4: Integration with backend_npu.c

Wire the NPU kernels into the existing `backend_npu.c` dispatcher:
1. Replace CPU fallback in `matmul_host()` with NPU call when available
2. Add shape matching logic (check if NPU kernel exists for shape)
3. Add NPU device detection to `coli_npu_init()`
4. Handle NPU errors gracefully (fall back to CPU on failure)

## Testing Checklist

- [ ] Device discovery works (already passing)
- [ ] Buffer objects work (already passing)
- [ ] HW context creation works (needs fix)
- [ ] Matmul CPU fallback works (already passing)
- [ ] Matmul NPU path works (Phase 2)
- [ ] AIE kernel compilation works (Phase 3, requires aiecompiler)
- [ ] Integration with backend_npu.c (Phase 4)
- [ ] End-to-end Ornith inference with NPU acceleration

## Known Issues

1. **HW context creation fails** — "Operation not supported" from ioctl
2. **aiecompiler not available** — cannot compile AIE kernels
3. **No root access** for udev rules or partition management
4. **NPU may be shared** with other processes (OS default partition)

## References

- Kernel driver header: `/usr/include/drm/amdxdna_accel.h`
- XRT install: `/opt/xrt/` (version 2.19.0)
- NPU device: `/dev/accel/accel0` (major 261)
- NPU PCI: `c3:00.1` (AMD 1022:17f0, rev 11)
- AIE array: 8 columns × 6 rows = 48 tiles, version 1.1
- Firmware: `/lib/firmware/amdnpu/17f0_11/`
