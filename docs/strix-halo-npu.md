# Strix Halo NPU — XDNA 2 Building Blocks

**Scope:** AMD Ryzen AI Max+ 395 "Strix Halo", XDNA 2 NPU, headless, inside
Podman. These are inference building blocks for one machine. There is no
portable variant and no CPU fallback anywhere in this path.

---

## 1. Hardware and kernel interface

| Property | Value |
|---|---|
| NPU | AMD XDNA 2 (AIE-2 array) |
| Array | 10 columns × 8 rows, partitionable |
| Throughput | ~50 TOPS INT8 (Ryzen AI Max+ 395 class) |
| Datapath | INT8 MAC. **No INT4 MAC** — int4 weights must be widened first. |
| Shapes | Fixed-shape kernels only. A kernel binary is bound to one (rows, inner, out) tuple. |
| Kernel driver | `amdxdna`, in-tree since Linux 6.14 |
| Device node | `/dev/accel/accel0` (override with `XDNA2_DEVICE`) |
| UAPI header | `<drm/amdxdna_accel.h>` from kernel headers ≥ 6.14 |
| Firmware | `/lib/firmware/amdnpu/17f0_11/` |

XRT is **not** required. This code talks to the kernel driver directly through
DRM ioctls. XRT remains relevant only if you use AMD's own runtime instead.

---

## 2. Layout

```
c/npu_kernels/
  xdna2_driver.{c,h}   DRM ioctl wrapper: device, hwctx, BOs, submit, wait, queries
  xdna2_xrt_driver.{c,h} XRT / XDNA shim control plane: availability probe only
  xdna2_matmul.{c,h}   Fixed-shape INT8 matmul runtime: kernel loading, DMA, dispatch
  npu_runtime.h        coli_npu_* integration surface used by c/backend_npu.c
  tests/test_npu.c     Device/runtime probe suite

vnni-int8-matmul/npu/
  xdna2_backend.{c,h}  strix_xdna2_* backend used by the VNNI building blocks

vnni-int8-matmul/tests/
  npu_device_test.c    NPU probe, generation check, no-fallback assertion
```

---

## 3. Execution model

```
strix_xdna2_matmul()
  └─ xdna2_matmul_int8()
       ├─ xdna2_find_kernel(shape)          fixed-shape lookup; NULL ⇒ hard fail
       ├─ xdna2_create_bo() × 3             activations, weights, output (AMDXDNA_BO_SHMEM)
       ├─ xdna2_map_bo()  + memcpy          host writes
       ├─ xdna2_sync_bo(TO_DEVICE)          cache maintenance
       ├─ build ERT packet in an AMDXDNA_BO_CMD buffer
       ├─ xdna2_submit_command()            DRM_IOCTL_AMDXDNA_EXEC_CMD → seq
       ├─ xdna2_wait_command()              DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT at point seq
       ├─ xdna2_sync_bo(FROM_DEVICE)
       └─ memcpy out + per-column scale
```

### Context lifetime

`xdna2_runtime_init()` must set up the client before any work is submitted:

```
xdna2_open_device()                       /dev/accel/accel0
  └─ xdna2_create_hwctx()
       ├─ xdna2_create_bo(AMDXDNA_BO_DEV_HEAP)   one heap per open fd, 64 MiB
       ├─ xdna2_map_bo(heap)                     mmap it 64 MiB-aligned: the
       │                                         driver records the heap user
       │                                         address only on mmap
       ├─ xdna2_create_bo(AMDXDNA_BO_CMD) × 2    UMQ + log buffers
       └─ DRM_IOCTL_AMDXDNA_CREATE_HWCTX         → handle, timeline syncobj
```

The device heap is not optional. `aie2_hwctx_init()` looks up
`client->dev_heap` and fails with `-ENOENT` if it is absent, and every
`AMDXDNA_BO_DEV` allocation (the DPU instruction streams) is sub-allocated from
it. `-ENOENT` here means "no device heap", not "ioctl unsupported" — an
unsupported ioctl returns `ENOTTY`.

Creating the heap is not enough: it must also be mapped. The driver sets
`heap->mem.userptr` in its mmap path (`amdxdna_gem_obj_mmap()` →
`amdxdna_hmm_register()`), and `amdxdna_drm_alloc_dev_bo()` rejects every
`AMDXDNA_BO_DEV` allocation with `-EINVAL` and logs "Invalid dev heap userptr"
while that address is still `AMDXDNA_INVALID_ADDR`. `aie2_hwctx_init()`
allocates its command buffers that way, so an unmapped heap makes
`CREATE_HWCTX` fail with `-EINVAL` even though the AIE partition is valid.

`GET_BO_INFO` reports "no address" as `AMDXDNA_INVALID_ADDR` (`~0`), not `0`,
for both `vaddr` (until the BO is mapped) and `map_offset` (always, for
`AMDXDNA_BO_DEV`). Treat that sentinel as "unset" or a mapping attempt hands
out `(void *)-1`.

Because a heap's user address exists only because this process mapped it,
`xdna2_create_hwctx()` mmaps the heap unconditionally — an address reported
before that is stale, not permission to skip the mapping — and then re-reads
`GET_BO_INFO` to confirm the driver recorded it. The driver sets
`mem.userptr` once, in `amdxdna_hmm_register()`, and never clears it, so a heap
that has no address after a successful mmap is a hard failure and is reported
as such rather than deferred to a bare `-EINVAL` from `CREATE_HWCTX`. When
`CREATE_HWCTX` does fail with `-EINVAL`, the heap's address is read again and
printed, which separates "the heap registration was lost" from "the firmware
refused the partition".

The mapping is also aligned to 64 MiB. `aie2_hwctx_init()` passes the heap's
user address to the firmware through `aie2_map_host_buf()`, which sends
`MSG_OP_MAP_HOST_BUFFER` in chunks of `dev_info->dev_mem_size` (`AIE2_DEVM_SIZE`,
64 MiB). The firmware refuses a base that is not a multiple of that chunk: the
mgmt message fails, the driver logs

```
aie2_send_mgmt_msg_wait: command opcode 0x106 failed, status 0x4000003
aie2_hwctx_init: Map host buffer failed, ret -22
amdxdna_drm_create_hwctx_ioctl: Init hwctx failed, ret -22
```

and `CREATE_HWCTX` returns `-EINVAL` with a correctly registered heap. The
kernel applies the same alignment when it allocates the heap itself
(`align = dev_info->dev_mem_size` for `AMDXDNA_BO_DEV_HEAP`), and both XRT and
ROCr reserve `2 × align - 1` bytes and `MAP_FIXED` the heap into the aligned
interior. `xdna2_map_bo_ex()` does the same. A plain `mmap()` is only page
aligned, so it met the requirement by accident: that is why the context came up
in some processes on the Strix Halo runner and not in others on the same
machine and the same kernel.

For the same reason the heap size must be a whole number of 64 MiB chunks, and
`XDNA2_HEAP_BYTES` is rejected rather than rounded when it is not.

Completion is reported on the per-context **timeline** syncobj returned by
`CREATE_HWCTX`: the kernel calls `drm_syncobj_add_point(syncobj, chain,
out_fence, seq)` for every job, so the `seq` returned by `EXEC_CMD` is the
timeline point to wait on. The mainline UAPI has no `WAIT_CMD` ioctl;
`DRM_IOCTL_AMDXDNA_WAIT_CMD` exists only in AMD's out-of-tree `xdna-driver`
headers, which is why building against distribution kernel headers cannot find
it.

### Kernel artifacts

AIE-2 cannot execute a dynamically shaped GEMM. The DPU instruction stream for
a given shape can only be produced ahead of time by AMD's AIE toolchain
(`aiecompiler` / `xclbinutil`), which is not part of the base XRT install.

Rather than guessing an ERT encoding in C, the runtime consumes a small
self-describing container so that the ERT opcode and CU mask travel with the
instruction blob it belongs to:

```
offset  size  field
0       4     magic       'XDN2'  (0x324E4458)
4       4     version     1
8       4     ert_opcode  ERT packet opcode for this kernel
12      4     cu_mask     compute-unit mask
16      4     rows
20      4     inner_dim
24      4     out_cols
28      4     fmt         1 = INT8
32      4     instr_size  instruction stream length in bytes
36      4     instr_words words the DPU should execute
40      …     instruction stream
```

All fields are little-endian. Artifacts are looked up as

```
${COLI_NPU_KERNEL_DIR:-npu/kernels}/matmul_int8_<rows>x<inner>x<out>.npukernel
```

or preloaded from a colon-separated list in `COLI_NPU_KERNELS`.

**Why the opcode is not hardcoded:** it depends on the kernel's calling
convention. Baking a guessed value into C produces a binary that looks correct
and silently submits malformed packets to the hardware — the exact failure mode
that made the Vulkan backend so hard to debug (see
`docs/vulkan_debug_attempts.md`).

**Which shapes to compile.** The set is enumerated, not discovered: see
`vnni-int8-matmul/sched/npu_shapes.c`. Qwen 3.6 MoE collapses to five distinct
`(inner, out)` pairs — attention q, k/v and o projections, the expert gate/up
projection and the expert down projection — and each is compiled at three row
tiles (256 for bulk prefill, 32 for small prefill, 1 for decode). That is 15
artifacts, which is what `XDNA2_MAX_KERNELS` is sized from; the number follows
from the enumeration rather than the other way round.

**How they are compiled.** `vnni-int8-matmul/npu/aie/` builds the artifacts
from a pinned IRON/`mlir-aie` + Peano toolchain packaged as a Podman image, and
`.github/workflows/npu-kernels.yml` runs that build in CI and publishes the
result as an artifact. AMD's proprietary `aiecompiler` has no public release
for XDNA 2, so the open toolchain is the only route; see
[`vnni-int8-matmul/npu/aie/README.md`](../vnni-int8-matmul/npu/aie/README.md)
for how to run it and for the gaps that remain. Two of those gaps matter here:
the decode row tile (`rows = 1`) has no valid tiling on the AIE int8 MAC and is
not built, and the DRM path does not yet register the compiled xclbin's
partition with the firmware.

**Which tensors can run on the NPU, and which cannot.** The AIE2P int8
microkernel multiplies in units of `8x8x8` (`kernels.mm(...).mac_dims` with the
device set to `npu2`; the AIE2 value of `4x8x8` is what IRON answers when no
device is set, and planning against it produces tilings the design rejects).
Everything below follows from that row granularity of 8 plus the row blocking
each design imposes:

| row tile | what it is | on the NPU? |
| --- | --- | --- |
| 256 | bulk prefill | yes — all five projections, whole-array design |
| 32 | small prefill | yes — all five projections, single-core design (the whole array needs at least 64 rows) |
| 1 | decode | **no** — a 1-row int8 matmul is not expressible on an 8-row MAC |

So the NPU is a *prefill* engine for this model. Every decode step, and every
prefill remainder that is not a whole multiple of 32 rows, has no artifact and
is refused with `-ENOENT`; `coli_choose_backend()` sees that through
`caps.npu_kernel_exists` and places the work on the CPU or the iGPU. Row counts
that are not multiples of 32 are decomposed by `coli_npu_plan_tiles()` into
`256`/`32`/`1` tiles, and the `1`-row tiles are exactly the part that does not
run — the remainder must be placed on another engine, not padded onto the NPU,
because a fixed-shape kernel reads exactly the rows it was compiled for and
padding would feed it uninitialised activations.

Tensor eligibility is otherwise per *projection*, not per weight: the five
`(inner, out)` pairs above cover q/k/v/o, the router gate and the expert
gate/up/down matrices, so any weight matching one of those pairs is eligible at
the supported row tiles. A projection outside the set is refused rather than
matched to the nearest compiled shape. Everything that is not an int8 matmul of
one of those pairs — attention itself, the softmax/top-k router arithmetic,
RMSNorm, RoPE, the SiLU/mul in the expert MLP and all dequantisation — has no
NPU kernel at all and is not a candidate for placement.

The compiled kernels accumulate in **int32** with no on-chip dequantisation, so
the readback path scales accumulators through `xdna2_dequant_i32()` instead of
reinterpreting the output buffer as floats — `fmt = 1` means "int8 operands,
int32 accumulators", not "int8 in, float out".

Any row count is then covered host-side by `coli_npu_plan_tiles()`, which
splits it greedily into those exact tiles, so every dispatch hits a compiled
kernel and the loader never has to widen a match.

**Where the shapes run** is a separate question, answered from measurement by
`coli_choose_backend()`; see `docs/placement-policy.md`.

---

### Control plane: DRM ioctls, optionally validated through XRT

The AMD stack drives the NPU through XRT plus the XDNA shim
(`libxrt_driver_xdna`), which is what teaches XRT to talk to
`/dev/accel/accel0`; see [`xdna_shim_guide.md`](xdna_shim_guide.md). This
repository dispatches with DRM ioctls instead, and that stays true:

* the `.npukernel` container carries a DPU instruction stream and an ERT
  opcode, not an `.xclbin` plus a kernel argument convention, so there is
  nothing for `xrtKernelOpen()` / `xrtRunStart()` to run;
* an XRT submit/wait path written against artifacts that do not exist would be
  a guessed ABI, which guardrail 2 and guardrail 5 below forbid.

What *is* implemented is the part that can be verified on hardware today:
`xdna2_xrt_driver.c` opens and closes the NPU through the official XRT C API
(`<xrt/xrt_device.h>`, compiled only when the build found XRT and defined
`COLI_NPU_XRT_AVAILABLE`) and reports the outcome. `XDNA2_DRIVER` selects the
policy:

| Value | Behaviour |
|---|---|
| `auto` (default) | probe XRT when compiled in, record the result, continue on DRM either way |
| `drm` | never touch XRT |
| `xrt` | require the XRT + shim stack; `xdna2_runtime_init()` returns `-ENOSYS` when it is missing |

`xrt` is the useful one on a production Strix Halo box: it turns "the shim was
never installed" into a loud failure instead of a silent difference in stack.
No value of `XDNA2_DRIVER` ever enables a CPU path.

---

## 4. Bugs fixed in this pass

The XDNA 2 driver wrapper had the same class of defect as the Vulkan shim:
hand-copied kernel structures and magic constants instead of the UAPI.

| Bug | Effect | Fix |
|---|---|---|
| `xdna2_query_firmware_version()` passed `buffer_size = 16` with `buffer` pointing at a single `uint32_t` | 12-byte stack buffer overflow in the caller | Use `struct amdxdna_drm_query_firmware_version` |
| `param = 15` for resource info | `DRM_AMDXDNA_QUERY_RESOURCE_INFO` is **12**; 15 is out of range | Use the named enum value |
| `xdna2_destroy_bo()` never issued `DRM_IOCTL_GEM_CLOSE` | one GEM handle leaked per allocation until the handle table filled | Issue `GEM_CLOSE` |
| UMQ and log BOs destroyed immediately after `CREATE_HWCTX` | context referenced BOs whose handles were already closed | Context owns both BOs for its lifetime |
| `ctx->syncobj_handle = exec_cmd.seq` | a 64-bit sequence number truncated into the syncobj handle, destroying it | Store `seq` in `ctx->last_seq` |
| `xdna2_wait_command()` returned success unconditionally | results read before the NPU finished | Wait on the hardware context's timeline syncobj at point `last_seq` |
| `CREATE_HWCTX` issued without a device heap | `aie2_hwctx_init()` returns `-ENOENT` ("The client dev heap object not exist"), so the NPU never initialises | `xdna2_create_hwctx()` allocates an `AMDXDNA_BO_DEV_HEAP` first and owns it for the context lifetime |
| Device heap created but never mapped | `amdxdna_drm_alloc_dev_bo()` logs "Invalid dev heap userptr" and `CREATE_HWCTX` fails with `-EINVAL` | `xdna2_create_hwctx()` mmaps the heap before creating the context |
| Device heap mapped at a page-aligned but not 64 MiB-aligned address | the firmware refuses `MSG_OP_MAP_HOST_BUFFER`, `aie2_hwctx_init()` logs "Map host buffer failed" and `CREATE_HWCTX` fails with `-EINVAL` in some processes and not others | `xdna2_map_bo_ex()` reserves `size + align - 1` bytes and `MAP_FIXED`s the heap into the 64 MiB-aligned interior |
| `GET_BO_INFO`'s `AMDXDNA_INVALID_ADDR` (`~0`) stored as a real address | `xdna2_map_bo()` returned `(void *)-1` as the host pointer for an unmapped BO | Normalise the sentinel to 0 and refuse to map a BO with no offset |
| `DRM_AMDXDNA_QUERY_RESOURCE_INFO` and `AMDXDNA_QOS_*` used unconditionally | build failure on every shipping kernel-headers package — both post-date Linux 6.18 | Build-system probe defines `COLI_HAVE_XDNA2_RESOURCE_INFO`; the QoS hint falls back to the driver default |
| XDNA 2 detected from the AIE metadata `version` field (`>= 2`) | Strix Halo firmware reports tile info version **1.1**, so the probe rejected the very silicon it targets | `xdna2_is_xdna2_hardware()` reads the accel node's PCI vendor/device/revision from sysfs (`0x1022:0x17f0`, rev `0x10`/`0x11`/`0x20`) |
| PCI vendor id checked against `0x1002` | that is the Radeon/ATI vendor id; the accel function reports `PCI_VENDOR_ID_AMD` (`0x1022`), so Strix Halo was rejected as "not XDNA 2" | Match the accel node on `0x1022`, the id the `amdxdna` driver itself binds to |
| `xdna2_runtime_init()` created the hwctx in a stack local | hwctx and device fd leaked on every init | Runtime owns the hwctx; `shutdown` releases it |
| Every matmul path ended in `strix_cpu_matmul()` | NPU benchmarks silently measured the CPU | Removed; unsupported shapes return `-ENOENT` |
| Local re-declarations of every ioctl struct | fragile against UAPI evolution | Use `struct amdxdna_drm_*` throughout |

---

## 5. Building and testing

Requirements: gcc, kernel headers ≥ 6.14 (for `<drm/amdxdna_accel.h>`), and the
`amdxdna` module loaded.

```bash
# VNNI building blocks, including the NPU backend
cd vnni-int8-matmul
make
make test

# Standalone NPU probe suite
cd c/npu_kernels
gcc -O2 -std=c11 -Wall -Wextra -D_GNU_SOURCE -I. \
    tests/test_npu.c xdna2_driver.c xdna2_matmul.c \
    -o test_npu -ldl -lpthread -lm
./test_npu
```

Headless, in Podman, on the Strix Halo host:

```bash
cd vnni-int8-matmul
./scripts/strix-halo-podman-test.sh
```

The harness passes `/dev/dri`, `/dev/kfd` and `/dev/accel/accel0` into the
container and mounts the repository root, because the NPU backend compiles the
driver from `c/npu_kernels/`. It passes **no** display server: the Vulkan
backend requests zero instance extensions and the NPU path never needed one.

### Environment variables

| Variable | Default | Purpose |
|---|---|---|
| `XDNA2_DEVICE` | `/dev/accel/accel0` | NPU accel node |
| `XDNA2_DRIVER` | `auto` | control plane: `auto`, `drm`, or `xrt` (see §3.4) |
| `XDNA2_XRT_DEVICE_INDEX` | `0` | device index passed to `xrtDeviceOpen()` |
| `XDNA2_VERBOSE` | unset | print AIE metadata, resource info and firmware version at init |
| `COLI_NPU_KERNEL_DIR` | `npu/kernels` | where `*.npukernel` artifacts are looked up |
| `COLI_NPU_KERNELS` | unset | colon-separated artifacts to preload |

### What a pass looks like

Without hardware, every device test reports an explicit SKIP and the host-side
arithmetic still runs:

```
TEST 1: NPU device discovery ... SKIP (NPU device not available)
TEST 5: Unsupported shape is rejected (no CPU fallback) ... SKIP (no NPU on this host)
TEST 6: int4 -> int8 weight expansion ... PASS
TEST 7: Kernel lookup returns NULL when nothing is loaded ... PASS
```

On Strix Halo, test 5 must report `PASS (rejected with -ENOENT)` when no kernel
artifact is present. If it ever passes by *computing a result*, a fallback has
been reintroduced and must be removed.

---

## 6. Guardrails — do not do these

1. **No CPU fallback, ever.** `xdna2_matmul_int8()` returns `-ENOENT` for an
   unsupported shape and `strix_xdna2_matmul()` returns 0. A fallback makes NPU
   benchmarks meaningless and hides missing kernels.
2. **No hand-copied UAPI structures.** Use `struct amdxdna_drm_*` from
   `<drm/amdxdna_accel.h>`. If the header is absent, fail the build — do not
   vendor a copy and do not re-declare the structs locally.
3. **No magic `param` numbers.** Always use `enum amdxdna_drm_get_param`
   constants. The previous `15` for resource info was simply wrong.
4. **No guessed ERT opcodes.** The opcode comes from the kernel artifact. If a
   real opcode is later confirmed against hardware, record the evidence here
   before hardcoding anything.
5. **No silent waits.** Every submission must be followed by
   `xdna2_wait_command()`. A wait that returns success without asking the kernel
   is worse than no wait at all — stubbing it out to make a build pass turns a
   loud failure into silently wrong numerics.
6. **No `/dev/dri/card1` for the NPU.** The NPU is an accel node
   (`/dev/accel/accel0`); `/dev/dri` is the iGPU.
7. **No INT4 MACs.** AIE-2 has no int4 datapath. Expand with
   `xdna2_dequant_int4()` and run an INT8 kernel.
8. **No XRT execution path without XRT artifacts.** `XDNA2_DRIVER=xrt`
   validates the official stack; it does not pretend to dispatch through it.
   An XRT run path may only be added together with real `.xclbin` artifacts.
9. **Do not widen the shape match.** Kernel lookup is an exact match on
   (rows, inner_dim, out_cols, fmt). Approximate matching would dispatch a
   kernel against a buffer it was not compiled for.

---

## 7. References

* Linux kernel UAPI: `include/uapi/drm/amdxdna_accel.h` — ioctl numbers,
  `enum amdxdna_drm_get_param`, `enum amdxdna_bo_type`, `enum amdxdna_cmd_type`.
* Linux kernel driver: `drivers/accel/amdxdna/` (in-tree since 6.14).
  `aie2_ctx.c` shows the device-heap requirement and `drm_syncobj_add_point()`
  signalling that makes `seq` a timeline point.
* Linux DRM UAPI: `include/uapi/drm/drm.h` — `DRM_IOCTL_GEM_CLOSE`,
  `DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT`, `struct drm_syncobj_timeline_wait`.
* AMD XDNA driver and XRT userspace: <https://github.com/amd/xdna-driver>,
  <https://github.com/amd/XRT>.
* `docs/vulkan_debug_attempts.md` — the iGPU counterpart, and the origin of the
  "never hand-write an ABI" rule applied here.
