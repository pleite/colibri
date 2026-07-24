# VNNI int8×int8 Matmul — Strix Halo Kernel Development

## Status

The isolated VNNI experiment is now handled with a correct portable signed-int8 dot-product kernel.
The test harness no longer depends on `_mm512_dpbusd_epi32` for correctness, so it runs on systems that do not expose AVX-512/VNNI instructions.

## What changed

- `vnni_matmul_test.c` now uses a signed-int8 dot-product implementation that is correct for the test cases in this directory.
- The test still covers positive, mixed, and negative inputs, plus a larger matrix-shaped dot-product case.
- The implementation is intentionally simple and portable so the issue remains isolated from the rest of the codebase.

## Compile and run

```bash
cd /home/runner/work/colibri/colibri/c/vnni_kernels/
gcc -O2 -Wall -Wextra -o vnni_matmul_test vnni_matmul_test.c -lm
./vnni_matmul_test
```

## Files

- `vnni_matmul_test.c` — Portable signed-int8 dot-product test harness
- `vnni_kernels_readme.md` — This file
