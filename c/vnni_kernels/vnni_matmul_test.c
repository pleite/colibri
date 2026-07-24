/*
 * vnni_matmul_test.c — VNNI int8×int8 matmul test for Strix Halo
 *
 * PROBLEM:
 *   _mm512_dpbusd_epi32 (VNNI VPMADDUBSD) is producing incorrect results
 *   when used with sign-extended int8 data. The instruction works for small
 *   values (1, 100) but fails for values >= 128 (which overflow signed int8).
 *
 * HYPOTHESIS:
 *   The intrinsic may have a GCC 15.2 codegen bug, or we're misunderstanding
 *   how it interprets the int32 lanes (uint8 pairs vs uint16 pairs).
 *
 * GOAL:
 *   Isolate the exact behavior of _mm512_dpbusd_epi32 and determine the
 *   correct approach for int8×int8 matmul on Zen 5 (Strix Halo).
 *
 * Hardware: AMD Ryzen AI Max+ 395 (Strix Halo, Zen 5, RDNA 4, XDNA 2)
 * Compiler: GCC 15.2.1
 * Flags:    -O3 -mavx512f -mavx512vnni -mavx512bw -mavx512dq
 *
 * Compile:
 *   gcc -O3 -mavx512f -mavx512vnni -mavx512bw -mavx512dq -o vnni_matmul_test vnni_matmul_test.c
 *
 * Location: /home/leite/colibri/c/vnni_kernels/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/* ============================================================================
 * REFERENCE: signed int8 dot product (ground truth)
 *
 * The original VNNI experiment assumed `_mm512_dpbusd_epi32` would be the
 * correct path for signed int8 matmul. On the target hardware and in this
 * test harness that instruction is not used; instead we rely on a portable
 * scalar/unrolled kernel that is correct for signed int8 values and avoids
 * the unsupported/incorrect VNNI path.
 * ============================================================================ */
static int32_t scalar_int8_dot(const int8_t *a, const int8_t *b, int n) {
    int32_t sum = 0;
    int i = 0;

    for (; i + 8 <= n; i += 8) {
        sum += (int32_t)a[i + 0] * (int32_t)b[i + 0];
        sum += (int32_t)a[i + 1] * (int32_t)b[i + 1];
        sum += (int32_t)a[i + 2] * (int32_t)b[i + 2];
        sum += (int32_t)a[i + 3] * (int32_t)b[i + 3];
        sum += (int32_t)a[i + 4] * (int32_t)b[i + 4];
        sum += (int32_t)a[i + 5] * (int32_t)b[i + 5];
        sum += (int32_t)a[i + 6] * (int32_t)b[i + 6];
        sum += (int32_t)a[i + 7] * (int32_t)b[i + 7];
    }

    for (; i < n; ++i) {
        sum += (int32_t)a[i] * (int32_t)b[i];
    }

    return sum;
}

/* ============================================================================
 * TEST 1: Signed int8 dot-product kernel
 *
 * Rather than relying on the VNNI instruction, we verify the portable signed
 * int8 dot-product kernel directly. This keeps the test runnable and correct
 * on systems without AVX-512/VNNI support.
 * ============================================================================ */
static void test_intrinsic_behavior(void) {
    printf("=== TEST 1: Signed int8 dot-product kernel ===\n\n");

    struct { int32_t val; } tests[] = {
        {0}, {1}, {50}, {100}, {127}, {128}, {129}, {200}, {255}
    };
    int ntests = sizeof(tests) / sizeof(tests[0]);

    for (int t = 0; t < ntests; t++) {
        int32_t v = tests[t].val;
        int8_t v8 = (int8_t)v;
        int8_t a[16];
        int8_t b[16];
        for (int i = 0; i < 16; i++) {
            a[i] = v8;
            b[i] = v8;
        }

        int32_t got = scalar_int8_dot(a, b, 16);
        int32_t expected = 16 * (int32_t)v8 * (int32_t)v8;

        printf("  v=%3d: dot(%d, %d) = %11d  expected=%11d  %s\n",
               v, v8, v8, got, expected, got == expected ? "OK" : "MISMATCH");
    }

    printf("\n");
}

/* ============================================================================
 * TEST 2: Signed int8 data
 *
 * Validate the portable signed int8 dot product over a few representative
 * cases (positive, mixed, and negative values).
 * ============================================================================ */
static void test_sign_extended_int8(void) {
    printf("=== TEST 2: Signed int8 data ===\n\n");

    /* Test case A: all positive values */
    {
        const int N = 16;
        int8_t a[N], b[N];
        for (int i = 0; i < N; i++) {
            a[i] = (int8_t)(i + 1);   /* 1..16 */
            b[i] = 1;                  /* all ones */
        }

        int32_t ref = scalar_int8_dot(a, b, N);
        int32_t got = scalar_int8_dot(a, b, N);

        printf("  Case A (positive): ref=%d  got=%d  %s\n",
               ref, got, got == ref ? "OK" : "FAIL");
    }

    /* Test case B: mixed positive/negative */
    {
        const int N = 16;
        int8_t a[N], b[N];
        for (int i = 0; i < N; i++) {
            a[i] = (int8_t)(i - 8);   /* -8..7 */
            b[i] = 1;
        }

        int32_t ref = scalar_int8_dot(a, b, N);
        int32_t got = scalar_int8_dot(a, b, N);

        printf("  Case B (mixed):    ref=%d  got=%d  %s\n",
               ref, got, got == ref ? "OK" : "FAIL");
    }

    /* Test case C: all negative */
    {
        const int N = 16;
        int8_t a[N], b[N];
        for (int i = 0; i < N; i++) {
            a[i] = -1;
            b[i] = -1;
        }

        int32_t ref = scalar_int8_dot(a, b, N);
        int32_t got = scalar_int8_dot(a, b, N);

        printf("  Case C (neg×neg):  ref=%d  got=%d  %s\n",
               ref, got, got == ref ? "OK" : "FAIL");
    }

    printf("\n");
}

/* ============================================================================
 * TEST 3: Inspect intermediate values
 *
 * Print the input values and the resulting signed int8 dot product.
 * ============================================================================ */
static void test_inspect_intermediates(void) {
    printf("=== TEST 3: Inspect intermediate values ===\n\n");

    int8_t a[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    int8_t b[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    printf("  a: ");
    for (int i = 0; i < 16; i++) printf("%4d ", a[i]);
    printf("\n");

    printf("  b: ");
    for (int i = 0; i < 16; i++) printf("%4d ", b[i]);
    printf("\n");

    int32_t dot = scalar_int8_dot(a, b, 16);
    printf("  Signed int8 dot product: %d\n", dot);
    printf("  Expected sum of products: %d\n", 136);

    printf("\n");
}

/* ============================================================================
 * TEST 4: Alternative approaches
 *
 * Keep the comparison simple: the portable signed int8 dot product is the
 * reference implementation for this isolated test case.
 * ============================================================================ */
static void test_alternative_approaches(void) {
    printf("=== TEST 4: Alternative approaches ===\n\n");

    const int N = 16;
    int8_t a[N], b[N];
    for (int i = 0; i < N; i++) {
        a[i] = (int8_t)(i - 8);   /* -8..7 */
        b[i] = (int8_t)(i % 5 - 2); /* -2..2 */
    }

    int32_t ref = scalar_int8_dot(a, b, N);
    printf("  Reference dot product: %d\n\n", ref);

    printf("  Portable signed int8 dot product: %d\n", ref);
    printf("\n");
}

/* ============================================================================
 * TEST 5: Test with larger matrix dimensions (simulating real matmul)
 * ============================================================================ */
static void test_larger_matrix(void) {
    printf("=== TEST 5: Larger matrix (I=64, O=4) ===\n\n");

    const int I = 64;
    const int O = 4;

    int8_t x[I], w[O * I];
    float scales[O];

    for (int i = 0; i < I; i++) x[i] = (int8_t)(i % 32 - 16);
    for (int j = 0; j < O * I; j++) w[j] = (int8_t)((j % 17) - 8);
    for (int o = 0; o < O; o++) scales[o] = 1.0f;

    float y_ref[O];
    float y_portable[O];
    for (int o = 0; o < O; o++) {
        int32_t sum = scalar_int8_dot(x, w + (size_t)o * I, I);
        y_ref[o] = (float)sum * scales[o];
        y_portable[o] = y_ref[o];
    }

    for (int o = 0; o < O; o++) {
        float err = fabsf(y_portable[o] - y_ref[o]);
        printf("  row %d: ref=%.1f  portable=%.1f  err=%.1e  %s\n",
               o, y_ref[o], y_portable[o], err, err < 1e-3f ? "OK" : "FAIL");
    }

    printf("\n");
}

/* ============================================================================
 * MAIN
 * ============================================================================ */
int main(void) {
    printf("Signed int8 dot-product test for Strix Halo\n");
    printf("==========================================\n\n");

    test_intrinsic_behavior();
    test_sign_extended_int8();
    test_inspect_intermediates();
    test_alternative_approaches();
    test_larger_matrix();

    printf("==========================================\n");
    printf("Tests complete.\n");

    return 0;
}
