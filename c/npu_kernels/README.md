# NPU Kernels — XDNA 2 (Strix Halo)

DRM-level runtime for the AMD XDNA 2 NPU in the Ryzen AI Max+ 395 "Strix Halo".
Strix Halo exclusive, headless, and with **no CPU fallback**.

The authoritative document for this backend is
[`docs/strix-halo-npu.md`](../../docs/strix-halo-npu.md) in the repository root:
device model, execution lifecycle, the `.npukernel` artifact format, the driver
bugs that were fixed, and the guardrails. This file is the directory-level quick
reference only.

## Hardware

| Property | Value |
|---|---|
| NPU | AMD XDNA 2 (AIE-2), Ryzen AI Max+ 395 |
| AIE array | 10 columns × 8 rows, partitionable |
| Memory | on-chip tile SRAM plus unified host DRAM |
| Throughput | ~50 TOPS INT8 |
| Datapath | INT8 MAC only — **no INT4 MAC** |
| Shapes | fixed-shape kernels only (AIE-2 tile ISA) |
| Device node | `/dev/accel/accel0` (override with `XDNA2_DEVICE`) |
| Kernel driver | `amdxdna`, in-tree since Linux 6.14 |

`/dev/dri/*` is the iGPU, not the NPU. XRT is not required; this code issues DRM
ioctls directly.

## Stack

```
c/backend_npu.c            coli_npu_* dispatcher
  xdna2_matmul.c           fixed-shape INT8 matmul: kernel loading, BOs, ERT packet, dispatch
    xdna2_driver.c         DRM ioctls: device, hwctx, BO alloc/map/sync, exec, wait, queries
      amdxdna (kernel)     partition management, DMA scheduling  — /dev/accel/accel0
        XDNA 2 hardware    AIE-2 tile array
```

## Files

| File | Purpose |
|---|---|
| `xdna2_driver.h/.c` | DRM ioctl wrapper: device, hardware context, buffer objects, submit, wait, queries |
| `xdna2_matmul.h/.c` | Fixed-shape INT8 matmul runtime, `.npukernel` loading, int4 → int8 expansion |
| `npu_runtime.h` | `coli_npu_*` integration surface used by `c/backend_npu.c` |
| `CMakeLists.txt` | Build; fails hard if `<drm/amdxdna_accel.h>` is missing, probes for post-6.18 UAPI additions |
| `tests/test_npu.c` | Device/runtime probe suite |

All ioctl structures come from `<drm/amdxdna_accel.h>` (kernel headers ≥ 6.14).
There is no vendored copy and no local re-declaration: if the header is absent
the build fails.

## Build and test

```bash
gcc -O2 -std=c11 -Wall -Wextra -D_GNU_SOURCE -I c/npu_kernels \
    c/npu_kernels/xdna2_driver.c \
    c/npu_kernels/xdna2_matmul.c \
    c/npu_kernels/tests/test_npu.c \
    -o c/npu_kernels/test_npu -ldl -lpthread -lm
./c/npu_kernels/test_npu
```

With CMake:

```bash
mkdir -p build-npu && cd build-npu
cmake ../c -DCOLI_NPU=ON
make test_npu
```

Check the device:

```bash
lsmod | grep amdxdna
ls -la /dev/accel/accel0
XDNA2_VERBOSE=1 ./c/npu_kernels/test_npu
```

Without hardware, the device tests report explicit SKIPs and the host-side
arithmetic tests still pass. A SKIP means "no NPU here" — it must never become a
PASS produced by computing the result somewhere else.

Some UAPI symbols (`DRM_AMDXDNA_QUERY_RESOURCE_INFO`, `AMDXDNA_QOS_*`,
`AMDXDNA_BO_SHARE`, `DRM_IOCTL_AMDXDNA_WAIT_CMD`) post-date Linux 6.18 or live
only in AMD's out-of-tree `xdna-driver` headers. The build probes for them; a
missing symbol degrades a *query*, never the correctness of a dispatch. Do not
stub a wait or a submit to make a build pass.

## Status

Implemented:

* DRM ioctl wrapper against the real UAPI — device, device heap, hwctx, BO
  alloc/map/sync, `EXEC_CMD`, `GET_INFO`, and completion via
  `DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT` on the context's timeline syncobj (the
  mainline UAPI has no `WAIT_CMD` ioctl).
* Fixed-shape INT8 matmul dataflow: kernel lookup, BO upload, cache maintenance,
  ERT packet construction, submission, wait, readback, per-column scaling.
* int4 → int8 weight expansion (`xdna2_dequant_int4`), host side.
* Hard rejection (`-ENOENT`) of shapes with no matching kernel.

Requires the AMD AIE toolchain:

* Producing the `.npukernel` artifacts themselves. `aiecompiler` is not part of
  the base XRT install; see <https://github.com/amd/XRT/releases>.
* Confirming the ERT opcode for the compiled kernels. The opcode currently
  travels inside the artifact precisely so that nothing has to be guessed in C.

## References

* `include/uapi/drm/amdxdna_accel.h` (Linux ≥ 6.14) — ioctls, `enum
  amdxdna_drm_get_param`, `enum amdxdna_bo_type`
* `include/uapi/drm/drm.h` — `DRM_IOCTL_GEM_CLOSE`,
  `DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT`
* `drivers/accel/amdxdna/` — the in-tree kernel driver
* <https://github.com/amd/xdna-driver>, <https://github.com/amd/XRT>
* Firmware: `/lib/firmware/amdnpu/17f0_11/`
