# VNNI int8×int8 Matmul — Debug Summary

## Status: BUG ISOLATED, FIX NEEDED

### What We Did
1. Created comprehensive test suite (`vnni_matmul_test.c`) with 5 test cases
2. Compiled and ran on Strix Halo hardware (GCC 15.2.1, Zen 5)
3. Identified the exact failure point: `_mm512_dpbusd_epi32` produces wrong results for values >= 128

### The Bug

**Intrinsic**: `_mm512_dpbusd_epi32(__m512i A, __m512i B, __m512i C)`

**Expected behavior**: Compute `A + sum(B[i] * C[i])` for 16 **unsigned** 8-bit pairs

**Actual behavior**: 
- Works correctly for values 0-127 (fits in signed int8)
- **Fails for values 128-255** (overflow signed int8 interpretation)

**Root cause hypothesis**: The intrinsic is treating the int32 lanes as containing **signed** values internally, not unsigned. When we add 128 to convert int8 to uint8, values >= 128 become negative in signed interpretation, causing the intrinsic to compute wrong results.

### Test Results Summary

```
TEST 1: Intrinsic behavior with controlled values
  v=  0: OK (0)
  v=  1: OK (16)
  v= 50: OK (40000)
  v=100: OK (160000)
  v=127: OK (258064)
  v=128: FAIL (-262144 vs expected 262144)
  v=129: FAIL (-262128 vs expected 266256)
  v=200: FAIL (-179200 vs expected 640000)
  v=255: FAIL (-4080 vs expected 1040400)

TEST 2: Sign-extended int8 data
  Case A (positive): FAIL (corrected=-528248 vs ref=136)
  Case B (mixed): FAIL (corrected=-264200 vs ref=-8)
  Case C (neg×neg): OK (corrected=16 vs ref=16)
```

**Key insight**: Case C (negative×negative) works because both operands are negative, so adding 128 keeps them in the 0-127 range where the intrinsic works correctly.

### Files in `/home/leite/colibri/c/vnni_kernels/`

1. **`vnni_matmul_test.c`** — Comprehensive test suite (compile and run to see output)
2. **`vnni_kernels_readme.md`** — Problem description and what we need from Copilot
3. **`vnni_matmul_test`** — Compiled binary (already built on Strix Halo)

### What Copilot Needs To Do

Please analyze the test output and determine:

1. **Is this a GCC 15.2 bug?**
   - Check GCC bug tracker for known VNNI issues
   - Try compiling with different flags (-O0, -O1, -O2)

2. **What's the correct approach?**
   - Should we use a different intrinsic?
   - Should we use AVX-512 FMA instead of VNNI?
   - Should we use a manual unrolled loop?

3. **Alternative implementations to consider:**
   - `_mm512_dpbusds_epi32` (signed byte version)
   - `_mm512_maddubs_epi16` + `_mm512_madd_epi16` (two-step)
   - AVX-512 FMA with int32 data (no VNNI)
   - Manual unrolled dot product with AVX-512

4. **Verify the fix**
   - Once we have a working approach, update the test to validate it
   - Ensure all test cases pass

### Next Steps

1. **Copilot analyzes the test output** and provides recommendations
2. **Implement the fix** based on Copilot's analysis
3. **Update `vnni_matmul_test.c`** with the working approach
4. **Validate** all test cases pass
5. **Benchmark** performance vs scalar reference

### Quick Reference

**Compile**:
```bash
cd /home/leite/colibri/c/vnni_kernels/
gcc -O3 -mavx512f -mavx512vnni -mavx512bw -mavx512dq -o vnni_matmul_test vnni_matmul_test.c
./vnni_matmul_test
```

**Expected output**: All tests should show "OK" or "Match: YES"

**Current output**: Tests fail for values >= 128

---

**Location**: `/home/leite/colibri/c/vnni_kernels/`
**Commit**: `06429dc feat(vnni): add int8×int8 matmul test for Strix Halo`
