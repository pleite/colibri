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
| `npu/aie/` | Containerised AIE toolchain that compiles the `.npukernel` artifacts |
| `tools/npu_shapes_list.c` | Prints the enumerated shape set, so the kernel build has one source of truth |
| `tools/placement_report.c` | Prints, per shape, which engine this host would use and why the others were refused |
| `sched/shape_profile.[ch]` | The measured per-shape, per-engine cost table |
| `sched/npu_shapes.[ch]` | Qwen 3.6 MoE shape enumeration and host-side row tiling |
| `sched/backend_placement.[ch]` | `coli_choose_backend()`, the one placement decision |
| `sched/moe_schedule.[ch]` | Router grouping, lane caps, plan execution, expert weight residency |
| `sched/engine_caps.[ch]` | Fills the placement capability snapshot by probing the real devices |
| `bench/backend_bench.c` | Sweeps the shapes and writes the measured table |
| `data/` | Where the measured table lives; see `data/README.md` |
| `kernel/vnni_matmul_test.c` | Small CPU-backend demo |
| `tests/test_backends.c` | Cross-backend correctness tests |
| `tests/vulkan_runtime_test.c` | Vulkan backend runtime test |
| `tests/vulkan_debug_harness.c` | Verbose Vulkan device enumeration and dispatch |
| `tests/npu_device_test.c` | NPU probe, AIE version check, no-fallback assertion |
| `tests/test_placement.c` | Profile table, shape set and placement policy, host-only |
| `tests/test_moe_schedule.c` | Expert grouping, lane caps and residency, host-only |
| `scripts/strix-halo-podman-test.sh` | Headless Podman harness for the Strix Halo host |
| `scripts/build-npu-kernels.sh` | Builds every enumerated NPU kernel in the toolchain container |

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

Placement is decided from a measured table, which only the Strix Halo machine
can produce:

```bash
make bench/backend_bench
./bench/backend_bench data/strix_halo_profile.csv
```

With no table present, `coli_choose_backend()` still decides, but labels the
decision `structural` so it cannot be mistaken for a measurement. See
[docs/placement-policy.md](../docs/placement-policy.md).

## NPU kernels

The `.npukernel` artifacts are compiled, not committed:

```bash
make npu-kernels          # toolchain image + every buildable shape
make npu-kernels-check    # host-only checks: shape set and container format
```

CI builds them too (`.github/workflows/npu-kernels.yml`) and uploads them as the
`npu-kernels` artifact. See [npu/aie/README.md](npu/aie/README.md) for the
toolchain, and for what is deliberately not built.

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
| `COLI_PLACEMENT` | pin an engine: `auto` (default), `cpu`, `gpu`, `npu` |
| `COLI_PLACEMENT_PROFILE` | measured table to load (default `data/strix_halo_profile.csv`) |
| `COLI_PLACEMENT_REQUIRE_PROFILE` | refuse any placement weaker than a measurement |
| `COLI_PLACEMENT_MIN_REUSE` | arithmetic-reuse threshold for the structural default |
| `COLI_MOE_CPU_LANES`, `COLI_MOE_GPU_LANES` | concurrent expert groups per engine |
| `COLI_BENCH_ITERS`, `COLI_BENCH_MAX_MIB`, `COLI_BENCH_OUTPUT` | benchmark sweep controls |

There is no `COLI_MOE_NPU_LANES`: one AIE partition means one hardware context.

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
6. Placement is measured, never asserted. A decision made without a measurement
   says so, and can be refused outright.

For a full runbook with test commands and edge-case notes, see
[docs/TESTING.md](docs/TESTING.md).
