# Testing and runtime notes

## Build and run

From the `vnni-int8-matmul` directory:

```bash
make
make test
```

The test target builds and runs both of the standalone harnesses:

```bash
./tests/test_backends
./tests/vulkan_runtime_test
```

## What the tests cover

- A correctness check for the CPU backend against a scalar reference implementation.
- A correctness check for the Vulkan backend against a scalar reference implementation.
- A correctness check for the XDNA2 wrapper against the same reference.
- A dedicated runtime test for the Vulkan backend that exercises the real wrapper entry point.
- Edge-case checks for invalid arguments and small shapes.

## Edge cases

The backends intentionally reject invalid inputs instead of silently producing partial results:

- `NULL` input, weights, output, or scales pointers
- non-positive `rows`, `inner_dim`, or `out_cols`
- zero-sized output shapes

The wrappers also handle small workloads such as:

- a single output column
- a single input row
- negative int8 values in the inputs and weights

## Runtime behavior on unsupported hosts

- The CPU backend is Strix Halo-specific and requires AVX-512 VNNI support. On unsupported hosts the CPU correctness test is skipped rather than attempting a generic fallback.
- The Vulkan backend is implemented as a real compute-wrapper path when a Vulkan loader and driver are available on the Strix Halo target. When they are not, the wrapper reports that the backend is unavailable.
- The XDNA2 wrapper remains a minimal shape-specific shim and is intended for the fixed `rows=1` / `out_cols=4` case.
