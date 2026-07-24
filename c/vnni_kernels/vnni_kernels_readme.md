# VNNI int8×int8 Matmul — Strix Halo Kernel Development

## Hardware
- **CPU**: AMD Ryzen AI Max+ 395 (Strix Halo, Zen 5) — 32 cores, AVX-512 + VNNI
- **iGPU**: RDNA 4 (gfx1151, Radeon 8060S)
- **NPU**: XDNA 2 (~30 TOPS INT8)
- **Memory**: 65GB unified RAM

## Problem Statement

We need to implement int8×int8 matrix multiplication using AVX-512 VNNI on Strix Halo.
The target intrinsic is `_mm512_dpbusd_epi32` (VPMADDUBSD), which computes:

```
A + sum(B[i] * C[i])  for 16 uint8 pairs
```

### Current Status: BROKEN

The VNNI path produces incorrect results. The issue is isolated to `_mm512_dpbusd_epi32`
when used with sign-extended int8 data that has been offset by +128 to convert to unsigned.

**Observed behavior:**
- With values 1..100: works correctly
- With values 128..255: produces wrong results
- The sign-extension (`_mm512_cvtepi8_epi32`) is verified correct
- The unsigned conversion (+128) is verified correct
- Only the VNNI dot product itself is broken

### Hypotheses

1. **GCC 15.2 codegen bug**: The intrinsic may have incorrect code generation for this
   specific instruction on Zen 5
2. **Data layout misunderstanding**: The intrinsic may interpret int32 lanes differently
   than documented (e.g., uint16 pairs instead of uint8 pairs)
3. **Sign extension issue**: The sign-extension may not be producing the expected values
4. **Accumulation overflow**: The 32-bit accumulator may be overflowing

## Test File

`vnni_matmul_test.c` contains 5 test cases:
1. Intrinsic behavior with controlled values (0, 1, 50, 100, 127, 128, 129, 200, 255)
2. Sign-extended int8 data (positive, mixed, negative×negative)
3. Intermediate value inspection
4. Alternative approaches
5. Larger matrix (I=64, O=4)

### Compile and Run

```bash
cd /home/leite/colibri/c/vnni_kernels/
gcc -O3 -mavx512f -mavx512vnni -mavx512bw -mavx512dq -o vnni_matmul_test vnni_matmul_test.c
./vnni_matmul_test
```

## What We Need From Copilot

Please analyze the test output and determine:

1. **What is `_mm512_dpbusd_epi32` actually computing?**
   - Is it 16 uint8 pairs? 16 uint16 pairs? Something else?
   - How does it interpret the int32 lanes?

2. **Is there a GCC 15.2 bug?**
   - Check GCC bug tracker for known issues with VNNI intrinsics
   - Try different optimization levels (-O0, -O1, -O2, -O3)

3. **What's the correct approach?**
   - Should we use a different intrinsic?
   - Should we use AVX-512 FMA instead of VNNI?
   - Should we use a manual unrolled loop?

4. **Alternative implementations to consider:**
   - `_mm512_dpbusds_epi32` (signed byte version)
   - `_mm512_maddubs_epi16` + `_mm512_madd_epi16` (two-step)
   - AVX-512 FMA with int32 data (no VNNI)
   - Manual unrolled dot product with AVX-512

## Files

- `vnni_matmul_test.c` — Comprehensive test suite
- `README.md` — This file

## Next Steps

1. Run `vnni_matmul_test.c` on Strix Halo
2. Analyze output with Copilot
3. Implement fix based on findings
4. Validate with full matmul test
5. Benchmark performance
