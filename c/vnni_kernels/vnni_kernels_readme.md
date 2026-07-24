# VNNI int8×int8 Matmul — Strix Halo Kernel Development

## Status

The standalone experiment is now a Strix-Halo-specific AVX-512/VNNI kernel. It targets AMD Zen 5 / Strix Halo CPUs and uses `_mm512_dpbusd_epi32` with the sign-flip trick so signed int8 dot-products remain correct while retaining the VNNI throughput advantage.

The harness intentionally does not provide a generic fallback for other hosts. On unsupported hardware it exits with a clear message instead of pretending to be portable.

## What changed

- `vnni_matmul_test.c` now uses a Strix-Halo-oriented AVX-512/VNNI path for signed-int8 dot products.
- The kernel is built around the VNNI instruction with the correct sign-flip transform for signed values.
- The test still validates positive, mixed, negative, and larger matrix-shaped dot-product cases against a scalar reference.
- The executable requires AVX-512 VNNI support and will refuse to run on other hosts.

## Compile and run

```bash
cd /home/runner/work/colibri/colibri/c/vnni_kernels/
gcc -O3 -Wall -Wextra -mavx512f -mavx512vnni -mavx512bw -mavx512dq -o vnni_matmul_test vnni_matmul_test.c -lm
./vnni_matmul_test
```

## Files

- `vnni_matmul_test.c` — Strix-Halo-specific AVX-512/VNNI signed-int8 dot-product test harness
- `vnni_kernels_readme.md` — This file
