# VNNI int8×int8 matmul — Strix Halo multi-backend prototype

This subproject is a small, self-contained implementation of int8 matmul for AMD Strix Halo. It focuses on three minimal backends:

- CPU: AVX-512 VNNI when available, otherwise a scalar fallback.
- GPU: a Vulkan-facing wrapper that dispatches through the CPU path when no loader is present.
- NPU: a fixed-shape XDNA2 wrapper that accepts one input row and four output columns.

## Layout

- `cpu/vnni_cpu_backend.[ch]` — optimized CPU kernel and scalar fallback.
- `gpu/vulkan_backend.[ch]` — Vulkan-fronted wrapper with a CPU fallback for portability.
- `npu/xdna2_backend.[ch]` — fixed-shape XDNA2 wrapper that uses the CPU path.
- `kernel/vnni_matmul_test.c` — tiny demo program that exercises the CPU backend.
- `tests/test_backends.c` — correctness tests for all three paths.

## Build and test

```bash
cd vnni-int8-matmul
make test
```

The build uses GCC with AVX-512 VNNI flags when available and still runs on hosts without the instruction set because the CPU backend falls back to scalar execution.

## Notes

The Vulkan and XDNA2 implementations are intentionally minimal and do not attempt to be a general-purpose abstraction. They are meant to be the smallest possible backend shims for this Strix Halo-focused prototype.
