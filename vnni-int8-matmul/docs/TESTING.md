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
./scripts/strix-halo-podman-test.sh
```

The container gets `/dev/dri`, `/dev/kfd` and `/dev/accel/accel0`, and the
repository root is mounted because the NPU backend compiles sources from
`c/npu_kernels/`. No display server is passed in and none is needed: the Vulkan
backend requests zero instance extensions. Do not add Xvfb, `DISPLAY` or
`XAUTHORITY` to this script — an earlier revision had them and they were pure
noise.

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
