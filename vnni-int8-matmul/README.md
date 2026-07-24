# VNNI int8×int8 matmul — Strix Halo multi-backend prototype

This subproject is a small, self-contained implementation of int8 matmul for AMD Strix Halo. It focuses on three minimal backends:

- CPU: AVX-512 VNNI on Strix Halo hardware.
- GPU: a Vulkan-facing wrapper that requires the Vulkan runtime on the Strix Halo target.
- NPU: a fixed-shape XDNA2 wrapper that accepts one input row and four output columns.

## Layout

- `cpu/vnni_cpu_backend.[ch]` — optimized CPU kernel for Strix Halo AVX-512 VNNI.
- `gpu/vulkan_backend.[ch]` — Vulkan-fronted wrapper for the Strix Halo target.
- `npu/xdna2_backend.[ch]` — fixed-shape XDNA2 wrapper that uses the CPU path.
- `kernel/vnni_matmul_test.c` — tiny demo program that exercises the CPU backend.
- `tests/test_backends.c` — correctness tests for all three paths.
- `tests/vulkan_runtime_test.c` — standalone runtime test that exercises the Vulkan backend directly.

## Build and test

```bash
cd vnni-int8-matmul
make test
```

The CPU backend is Strix Halo-specific and requires AVX-512 VNNI support. The default build uses the required flags for that target:

```bash
make test
```

If you are building on a non-Strix-Halo host, the runtime tests fail immediately when the required GPU/CPU backend cannot initialize.

## Notes

The Vulkan and XDNA2 implementations are intentionally minimal and do not attempt to be a general-purpose abstraction. They are meant to be the smallest possible backend shims for this Strix Halo-focused prototype.

For a full runbook with test commands and edge-case notes, see [docs/TESTING.md](docs/TESTING.md).
