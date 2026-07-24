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
#include <immintrin.h>
#include <math.h>

/* ============================================================================
 * REFERENCE: scalar int8 matmul (ground truth)
 * ============================================================================ */
static int32_t scalar_int8_dot(const int8_t *a, const int8_t *b, int n) {
    int32_t sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (int32_t)a[i] * (int32_t)b[i];
    }
    return sum;
}

/* ============================================================================
 * TEST 1: Intrinsic behavior with controlled values
 *
 * We test _mm512_dpbusd_epi32 with various uint8-range values to understand
 * exactly what it computes.
 * ============================================================================ */
static void test_intrinsic_behavior(void) {
    printf("=== TEST 1: Intrinsic behavior with controlled values ===\n\n");

    struct { int32_t val; } tests[] = {
        {0}, {1}, {50}, {100}, {127}, {128}, {129}, {200}, {255}
    };
    int ntests = sizeof(tests) / sizeof(tests[0]);

    for (int t = 0; t < ntests; t++) {
        int32_t v = tests[t].val;

        /* Set all 16 int32 lanes to v (only low byte is non-zero for v < 256) */
        __m512i a = _mm512_setzero_epi32();
        __m512i b = _mm512_set1_epi32(v);
        __m512i c = _mm512_set1_epi32(v);

        __m512i r = _mm512_dpbusd_epi32(a, b, c);
        int32_t got = _mm512_reduce_add_epi32(r);

        /* Expected: 16 * v * v (if the intrinsic treats each int32 lane as one uint8) */
        int32_t expected = 16 * v * v;

        printf("  v=%3d: dpbusd(zero, %d, %d) = %11d  expected=%11d  %s\n",
               v, v, v, got, expected, got == expected ? "OK" : "MISMATCH");
    }

    printf("\n");
}

/* ============================================================================
 * TEST 2: Sign-extended int8 data through the VNNI path
 *
 * Load int8 data as __m128i, sign-extend to __m512i, add 128 to convert
 * to unsigned, then run VNNI dot product.
 * ============================================================================ */
static void test_sign_extended_int8(void) {
    printf("=== TEST 2: Sign-extended int8 data ===\n\n");

    /* Test case A: all positive values */
    {
        const int N = 16;
        int8_t a[N], b[N];
        for (int i = 0; i < N; i++) {
            a[i] = (int8_t)(i + 1);   /* 1..16 */
            b[i] = 1;                  /* all ones */
        }

        int32_t ref = scalar_int8_dot(a, b, N);

        /* VNNI path */
        __m128i a_lo = _mm_loadu_si128((const __m128i *)a);
        __m128i b_lo = _mm_loadu_si128((const __m128i *)b);

        /* Sign-extend to __m512i */
        __m512i a_512 = _mm512_cvtepi8_epi32(a_lo);
        __m512i b_512 = _mm512_cvtepi8_epi32(b_lo);

        /* Convert to unsigned (add 128) */
        __m512i a_u = _mm512_add_epi32(a_512, _mm512_set1_epi32(128));
        __m512i b_u = _mm512_add_epi32(b_512, _mm512_set1_epi32(128));

        __m512i acc = _mm512_setzero_epi32();
        acc = _mm512_dpbusd_epi32(acc, b_u, a_u);
        int32_t vnni_result = _mm512_reduce_add_epi32(acc);

        /* Correction: sum(signed) = vnni - 128*sum(a) - 128*sum(b) - 128*128*N */
        int32_t sum_a = 0, sum_b = 0;
        for (int i = 0; i < N; i++) { sum_a += a[i]; sum_b += b[i]; }
        int32_t correction = 128 * sum_a + 128 * sum_b + 128 * 128 * N;
        int32_t corrected = vnni_result - correction;

        printf("  Case A (positive): ref=%d  vnni_raw=%d  corrected=%d  %s\n",
               ref, vnni_result, corrected, corrected == ref ? "OK" : "FAIL");
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

        __m128i a_lo = _mm_loadu_si128((const __m128i *)a);
        __m128i b_lo = _mm_loadu_si128((const __m128i *)b);

        __m512i a_512 = _mm512_cvtepi8_epi32(a_lo);
        __m512i b_512 = _mm512_cvtepi8_epi32(b_lo);

        __m512i a_u = _mm512_add_epi32(a_512, _mm512_set1_epi32(128));
        __m512i b_u = _mm512_add_epi32(b_512, _mm512_set1_epi32(128));

        __m512i acc = _mm512_setzero_epi32();
        acc = _mm512_dpbusd_epi32(acc, b_u, a_u);
        int32_t vnni_result = _mm512_reduce_add_epi32(acc);

        int32_t sum_a = 0, sum_b = 0;
        for (int i = 0; i < N; i++) { sum_a += a[i]; sum_b += b[i]; }
        int32_t correction = 128 * sum_a + 128 * sum_b + 128 * 128 * N;
        int32_t corrected = vnni_result - correction;

        printf("  Case B (mixed):    ref=%d  vnni_raw=%d  corrected=%d  %s\n",
               ref, vnni_result, corrected, corrected == ref ? "OK" : "FAIL");
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

        __m128i a_lo = _mm_loadu_si128((const __m128i *)a);
        __m128i b_lo = _mm_loadu_si128((const __m128i *)b);

        __m512i a_512 = _mm512_cvtepi8_epi32(a_lo);
        __m512i b_512 = _mm512_cvtepi8_epi32(b_lo);

        __m512i a_u = _mm512_add_epi32(a_512, _mm512_set1_epi32(128));
        __m512i b_u = _mm512_add_epi32(b_512, _mm512_set1_epi32(128));

        __m512i acc = _mm512_setzero_epi32();
        acc = _mm512_dpbusd_epi32(acc, b_u, a_u);
        int32_t vnni_result = _mm512_reduce_add_epi32(acc);

        int32_t sum_a = 0, sum_b = 0;
        for (int i = 0; i < N; i++) { sum_a += a[i]; sum_b += b[i]; }
        int32_t correction = 128 * sum_a + 128 * sum_b + 128 * 128 * N;
        int32_t corrected = vnni_result - correction;

        printf("  Case C (neg×neg):  ref=%d  vnni_raw=%d  corrected=%d  %s\n",
               ref, vnni_result, corrected, corrected == ref ? "OK" : "FAIL");
    }

    printf("\n");
}

/* ============================================================================
 * TEST 3: Inspect intermediate values
 *
 * Print the actual int32 values after sign-extension and unsigned conversion
 * to see exactly what the intrinsic is operating on.
 * ============================================================================ */
static void test_inspect_intermediates(void) {
    printf("=== TEST 3: Inspect intermediate values ===\n\n");

    int8_t a[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    int8_t b[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    __m128i a_lo = _mm_loadu_si128((const __m128i *)a);
    __m128i b_lo = _mm_loadu_si128((const __m128i *)b);

    __m512i a_512 = _mm512_cvtepi8_epi32(a_lo);
    __m512i b_512 = _mm512_cvtepi8_epi32(b_lo);

    int32_t av[16], bv[16];
    _mm512_storeu_si512(av, a_512);
    _mm512_storeu_si512(bv, b_512);

    printf("  After sign-extend (a): ");
    for (int i = 0; i < 16; i++) printf("%4d ", av[i]);
    printf("\n");

    printf("  After sign-extend (b): ");
    for (int i = 0; i < 16; i++) printf("%4d ", bv[i]);
    printf("\n");

    __m512i a_u = _mm512_add_epi32(a_512, _mm512_set1_epi32(128));
    __m512i b_u = _mm512_add_epi32(b_512, _mm512_set1_epi32(128));

    int32_t au[16], bu[16];
    _mm512_storeu_si512(au, a_u);
    _mm512_storeu_si512(bu, b_u);

    printf("  After +128 (a):      ");
    for (int i = 0; i < 16; i++) printf("%4d ", au[i]);
    printf("\n");

    printf("  After +128 (b):      ");
    for (int i = 0; i < 16; i++) printf("%4d ", bu[i]);
    printf("\n");

    /* Now check: what does dpbusd compute? */
    __m512i acc = _mm512_setzero_epi32();
    acc = _mm512_dpbusd_epi32(acc, b_u, a_u);
    int32_t dot = _mm512_reduce_add_epi32(acc);

    printf("  VNNI dot product:    %d\n", dot);

    /* Expected unsigned dot */
    int32_t expected_unsigned = 0;
    for (int i = 0; i < 16; i++) {
        expected_unsigned += au[i] * bu[i];
    }
    printf("  Expected unsigned:   %d\n", expected_unsigned);
    printf("  Difference:          %d\n", dot - expected_unsigned);

    printf("\n");
}

/* ============================================================================
 * TEST 4: Try alternative approaches
 *
 * Approach A: Use _mm512_dpbusd_epi32 with uint8 packed in int32 (low byte only)
 * Approach B: Use AVX-512 FMA with int32 data (no VNNI)
 * Approach C: Manual unrolled int8 dot product with AVX-512
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

    /* Approach A: VNNI with uint8 packed in low byte of int32 */
    {
        /* Build vectors directly from uint8 values */
        int32_t av[N], bv[N];
        for (int i = 0; i < N; i++) {
            av[i] = (uint8_t)a[i];
            bv[i] = (uint8_t)b[i];
        }
        __m512i va = _mm512_loadu_si512(av);
        __m512i vb = _mm512_loadu_si512(bv);

        __m512i acc = _mm512_setzero_epi32();
        acc = _mm512_dpbusd_epi32(acc, vb, va);
        int32_t got = _mm512_reduce_add_epi32(acc);

        /* Expected unsigned dot */
        int32_t exp = 0;
        for (int i = 0; i < N; i++) exp += (uint8_t)a[i] * (uint8_t)b[i];

        printf("  Approach A (VNNI uint8):\n");
        printf("    VNNI result:      %d\n", got);
        printf("    Expected unsigned: %d\n", exp);
        printf("    Difference:       %d\n", got - exp);
    }

    printf("\n");
}

/* ============================================================================
 * TEST 5: Test with larger matrix dimensions (simulating real matmul)
 * ============================================================================ */
static void test_larger_matrix(void) {
    printf("=== TEST 5: Larger matrix (I=64, O=4) ===\n\n");

    const int I = 64;  /* Must be multiple of 16 */
    const int O = 4;

    int8_t x[I], w[O * I];
    float scales[O];

    /* Fill with known values */
    for (int i = 0; i < I; i++) x[i] = (int8_t)(i % 32 - 16);
    for (int j = 0; j < O * I; j++) w[j] = (int8_t)((j % 17) - 8);
    for (int o = 0; o < O; o++) scales[o] = 1.0f;

    /* Scalar reference */
    float y_ref[O];
    for (int o = 0; o < O; o++) {
        int32_t sum = 0;
        for (int i = 0; i < I; i++) {
            sum += (int32_t)x[i] * (int32_t)w[o * I + i];
        }
        y_ref[o] = (float)sum * scales[o];
    }

    /* VNNI path */
    float y_vnni[O];

    /* Precompute sum(x) */
    __m512i sum_x = _mm512_setzero_epi32();
    for (int i = 0; i < I; i += 16) {
        __m128i x_lo = _mm_loadu_si128((const __m128i *)(x + i));
        __m512i x_512 = _mm512_cvtepi8_epi32(x_lo);
        sum_x = _mm512_add_epi32(sum_x, x_512);
    }
    int32_t sum_x_val = _mm512_reduce_add_epi32(sum_x);

    for (int o = 0; o < O; o++) {
        const int8_t *wrow = w + (size_t)o * I;

        __m512i sum_w = _mm512_setzero_epi32();
        __m512i acc = _mm512_setzero_epi32();

        for (int i = 0; i < I; i += 16) {
            __m128i w_lo = _mm_loadu_si128((const __m128i *)(wrow + i));
            __m128i x_lo = _mm_loadu_si128((const __m128i *)(x + i));

            __m512i w_512 = _mm512_cvtepi8_epi32(w_lo);
            __m512i x_512 = _mm512_cvtepi8_epi32(x_lo);

            __m512i w_u = _mm512_add_epi32(w_512, _mm512_set1_epi32(128));
            __m512i x_u = _mm512_add_epi32(x_512, _mm512_set1_epi32(128));

            acc = _mm512_dpbusd_epi32(acc, w_u, x_u);
            sum_w = _mm512_add_epi32(sum_w, w_512);
        }

        int32_t dot = _mm512_reduce_add_epi32(acc);
        int32_t sum_w_val = _mm512_reduce_add_epi32(sum_w);
        int32_t correction = 128 * sum_x_val + 128 * sum_w_val + 262144 * (I / 16);
        int32_t result = dot - correction;
        y_vnni[o] = (float)result * scales[o];
    }

    /* Compare */
    for (int o = 0; o < O; o++) {
        float err = fabsf(y_vnni[o] - y_ref[o]);
        printf("  row %d: ref=%.1f  vnni=%.1f  err=%.1e  %s\n",
               o, y_ref[o], y_vnni[o], err, err < 1e-3f ? "OK" : "FAIL");
    }

    printf("\n");
}

/* ============================================================================
 * MAIN
 * ============================================================================ */
int main(void) {
    printf("VNNI int8×int8 Matmul Test for Strix Halo\n");
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
