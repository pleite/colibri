# VNNI int8 matmul — Strix Halo building blocks

Self-contained INT8 matmul building blocks for AMD Ryzen AI Max+ 395
"Strix Halo". Three backends, one machine, headless, no portability layer and
**no fallbacks**: each backend either runs on its own hardware or reports why it
cannot.

| Backend | Hardware | Reports when unusable |
|---|---|---|
| CPU | AVX-512 VNNI (Zen 5) | `avx512-vnni-unavailable` |
| GPU | RADV GFX1151 iGPU, headless Vulkan compute | `strix_vulkan_failure_reason()` |
| NPU | XDNA 2 via the `amdxdna` accel driver | `strix_xdna2_failure_reason()` |

## Layout

| Path | Purpose |
|---|---|
| `cpu/vnni_cpu_backend.[ch]` | AVX-512 VNNI kernel (`_mm512_dpbusd_epi32`) |
| `gpu/vulkan_backend.[ch]` | Headless Vulkan compute, Strix Halo exclusive |
| `gpu/vulkan_dispatch.h` | Dispatch tables built only from `<vulkan/vulkan_core.h>` |
| `gpu/comp.comp`, `gpu/comp.spv` | GEMM compute shader source and compiled SPIR-V |
| `npu/xdna2_backend.[ch]` | XDNA 2 NPU backend, driving `c/npu_kernels/` |
| `kernel/vnni_matmul_test.c` | Small CPU-backend demo |
| `tests/test_backends.c` | Cross-backend correctness tests |
| `tests/vulkan_runtime_test.c` | Vulkan backend runtime test |
| `tests/vulkan_debug_harness.c` | Verbose Vulkan device enumeration and dispatch |
| `tests/npu_device_test.c` | NPU probe, AIE version check, no-fallback assertion |
| `scripts/strix-halo-podman-test.sh` | Headless Podman harness for the Strix Halo host |

## Build requirements

* gcc with AVX-512 VNNI support
* Vulkan headers and loader — `vulkan-headers` / `libvulkan-dev`
* Linux kernel headers ≥ 6.14, for `<drm/amdxdna_accel.h>`
* `glslangValidator`, only to regenerate `gpu/comp.spv` (`make shader`)

## Build and test

```bash
make
make test
```

On the Strix Halo host, headless, in a container:

```bash
./scripts/strix-halo-podman-test.sh
```

On any other host the suite still builds and runs; every hardware-bound test
reports an explicit SKIP with the reason, and nothing crashes.

## Diagnostics

```bash
VNNI_VULKAN_DEBUG=1 ./tests/vulkan_debug_harness   # enumerate and explain device selection
XDNA2_VERBOSE=1     ./tests/npu_device_test        # AIE metadata, resource info, firmware
```

| Variable | Purpose |
|---|---|
| `VNNI_VULKAN_DEBUG` | verbose Vulkan device enumeration and dispatch resolution |
| `VNNI_VULKAN_SHADER` | override the path to `comp.spv` |
| `VNNI_STRIX_DEVICE_NAME` | allowlist one extra Strix Halo device-name substring |
| `XDNA2_DEVICE` | NPU accel node (default `/dev/accel/accel0`) |
| `XDNA2_VERBOSE` | dump NPU device info at init |
| `COLI_NPU_KERNEL_DIR` | where `*.npukernel` artifacts are found |
| `COLI_NPU_KERNELS` | colon-separated artifacts to preload |

`VNNI_STRIX_DEVICE_NAME` can only add an accepted device name. The AMD vendor ID
and `INTEGRATED_GPU` checks are unconditional, so it cannot be used to run this
code on a discrete GPU or a software rasteriser.

## Design rules

These are load-bearing. See `docs/vulkan_debug_attempts.md` and
`docs/strix-halo-npu.md` in the repository root for the full rationale and the
bugs that motivated them.

1. Never hand-write a Vulkan or kernel UAPI type. Include the official header or
   fail the build.
2. `dlsym` is used for `vkGetInstanceProcAddr` and nothing else.
3. Headless: zero Vulkan instance extensions, no display server, no Xvfb.
4. No fallbacks. Not to lavapipe, not to a discrete GPU, not to the CPU.
5. Fixed-shape NPU kernels only, matched exactly on (rows, inner, out, fmt).

For a full runbook with test commands and edge-case notes, see
[docs/TESTING.md](docs/TESTING.md).
