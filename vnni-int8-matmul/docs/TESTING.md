# Testing and runtime notes

## Build and run

From the `vnni-int8-matmul` directory:

```bash
make
make test
```

`make test` builds and runs three harnesses:

```bash
./tests/test_backends        # cross-backend correctness
./tests/vulkan_runtime_test  # Vulkan backend end to end
./tests/npu_device_test      # NPU probe and no-fallback assertion
```

`make shader` regenerates `gpu/comp.spv` from `gpu/comp.comp`; it needs
`glslangValidator` and is only required if the shader source changes.

## Headless testing on the Strix Halo host

```bash
make podman-test        # or ./scripts/strix-halo-podman-test.sh
make podman-debug-test  # same test flow + full host/container debug snapshots
make podman-debug-benchmark  # on-demand NPU benchmark flow + full debug
```

The script builds `Dockerfile.strix-halo-test` on top of the community Strix
Halo toolbox image and runs the suite inside it. The toolbox image is an
inference runtime, not a build environment: it ships gcc but no `ld`, no kernel
UAPI headers and no Vulkan headers, so building this tree in it fails with
`collect2: fatal error: cannot find 'ld'`. The harness image installs
`binutils`, `kernel-headers` and `vulkan-headers` and verifies all three at
image-build time. Never paper over this with `ln -sf /usr/bin/ld.bfd
/usr/bin/ld` inside a test step.

The container gets `/dev/dri`, `/dev/kfd` and `/dev/accel/accel0`, and the
repository root is mounted because the NPU backend compiles sources from
`c/npu_kernels/`. No display server is passed in and none is needed: the Vulkan
backend requests zero instance extensions. Do not add Xvfb, `DISPLAY` or
`XAUTHORITY` to this script — an earlier revision had them and they were pure
noise.

Useful overrides:

| Variable | Effect |
|---|---|
| `BASE_IMAGE` | upstream toolbox image the harness is layered on |
| `CONTAINER_IMAGE` | run a prebuilt image as-is instead of building one |
| `SKIP_NPU=1` | do not pass the NPU device through |
| `REQUIRE_NPU=1` | fail instead of warning when `/dev/accel/accel0` is absent |
| `XDNA2_HEAP_BYTES` | size of the NPU device heap (a multiple of 64 MiB, at most 64 MiB) |
| `COLI_CAPTURE_DEBUG=1` | write full host/container diagnostics under `vnni-int8-matmul/debug/` |

Use `REQUIRE_NPU=1` on the Strix Halo runner so an unloaded `amdxdna` module is
reported as a red run rather than hidden behind three SKIPs.

## CI workflow for local self-hosted debug

GitHub Actions workflow: `/home/runner/work/colibri/colibri/.github/workflows/vnni-local-runner-debug.yml`

- Runs on the local self-hosted runner (`self-hosted`, `linux`, `x64`).
- Supports `workflow_dispatch` with `mode=test|benchmark`.
- Captures full host/container debug snapshots (kernel, modules, headers, libraries, devices, toolchain) and uploads them as artifacts.

## What the tests cover

* CPU, Vulkan and XDNA 2 results checked against a scalar int8 reference.
* A dedicated Vulkan runtime test that exercises the real dispatch path.
* Argument-validation and small-shape edge cases.
* An explicit assertion that an unsupported NPU shape is **rejected** rather than
  computed on the CPU.
* int4 → int8 weight expansion, which is host-side and always runs.

## Edge cases

The backends reject invalid input instead of producing partial results:

* `NULL` input, weights, output or scales pointers
* non-positive `rows`, `inner_dim` or `out_cols`
* zero-sized output shapes

and handle a single output column, a single input row, and negative int8 values.

## Behaviour on a host that is not Strix Halo

Every hardware-bound test reports an explicit SKIP with a reason and the suite
exits 0. Nothing crashes and nothing is silently computed somewhere else:

```
CPU backend SKIP (requires AVX-512 VNNI on Strix Halo)
Vulkan backend SKIP (vkCreateInstance failed)
XDNA2 backend SKIP (cannot open /dev/accel/accel0 ...)
int4 -> int8 expansion OK
```

A SKIP means "this silicon is absent". If any of these ever turns into a PASS on
a non-Strix host, a fallback has been reintroduced and must be removed.

## Diagnostics

```bash
VNNI_VULKAN_DEBUG=1 ./tests/vulkan_debug_harness
XDNA2_VERBOSE=1     ./tests/npu_device_test
```

## Reading NPU failures correctly

`errno` values from the `amdxdna` ioctls are specific; do not generalise them
into "the kernel does not support this yet".

| Symptom | What it actually means |
|---|---|
| `CREATE_HWCTX` → `ENOENT` | this client has no `AMDXDNA_BO_DEV_HEAP`; the heap must be allocated before the context |
| `CREATE_HWCTX` → `ENOTTY` | the ioctl really is absent — wrong device node, or a kernel older than 6.14 |
| `CREATE_HWCTX` → `ENODEV` | the DRM device is being unbound |
| `open(/dev/accel/accel0)` → `ENOENT` | `amdxdna` is not loaded, or the node was not passed into the container |
| `open(...)` → `EACCES` | the node is present but the container user is not in its group |
| `DRM_AMDXDNA_QUERY_RESOURCE_INFO` unavailable | the build ran against kernel headers older than the post-6.18 UAPI that added it |

A compile error for `DRM_IOCTL_AMDXDNA_WAIT_CMD`, `AMDXDNA_BO_SHARE` or
`AMDXDNA_QOS_*` is a *header* problem, not a kernel capability problem: those
symbols live in AMD's out-of-tree `xdna-driver` headers or in post-6.18
mainline. The fix is a build-time probe or the mainline equivalent, never a
stub that returns success.
