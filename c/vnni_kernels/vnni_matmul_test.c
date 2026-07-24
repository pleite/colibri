/*
 * vnni_matmul_test.c — Strix Halo optimized int8×int8 matmul test
 *
 * The signed-int8 dot-product is the core kernel for this test. On AMD Zen 5
 * (Strix Halo) the fastest path is AVX-512 VNNI via `_mm512_dpbusd_epi32`.
 * The important detail is the sign-flip trick: use `abs(w)` as the unsigned
 * operand and flip the sign of the `x` lane when `w` is negative. This keeps
 * the computation correct for signed int8 values while preserving the VNNI
 * throughput advantage.
 *
 * Build:
 *   gcc -O3 -mavx512f -mavx512vnni -mavx512bw -mavx512dq -o vnni_matmul_test vnni_matmul_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#if defined(__AVX512VNNI__) && defined(__AVX512BW__)
#include <immintrin.h>
#endif

/* ==========================================================================
 * REFERENCE: scalar signed-int8 dot product (ground truth)
 * ========================================================================== */
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

#if defined(__AVX512VNNI__) && defined(__AVX512BW__)
static inline int32_t vnni_int8_dot(const int8_t *a, const int8_t *b, int n) {
    int32_t sum = 0;
    int i = 0;

    /* Strix Halo / Zen 5 has AVX-512 VNNI; use a 64-byte block size to keep
     * the dpbusd datapath fully occupied. The sign-flip trick makes the
     * operation valid for signed int8 weights and activations. */
    __m512i acc = _mm512_setzero_si512();
    const __m512i zero = _mm512_setzero_si512();

    for (; i + 64 <= n; i += 64) {
        __m512i av = _mm512_loadu_si512((const void *)(a + i));
        __m512i bv = _mm512_loadu_si512((const void *)(b + i));
        __mmask64 neg = _mm512_movepi8_mask(bv);
        __m512i xs = _mm512_mask_sub_epi8(av, neg, zero, av);
        acc = _mm512_dpbusd_epi32(acc, _mm512_abs_epi8(bv), xs);
    }

    sum = _mm512_reduce_add_epi32(acc);

    for (; i < n; ++i) {
        sum += (int32_t)a[i] * (int32_t)b[i];
    }

    return sum;
}
#endif

static const char *dot_kernel_name(void) {
#if defined(__AVX512VNNI__) && defined(__AVX512BW__)
    return "avx512-vnni";
#else
    return "scalar";
#endif
}

static int32_t signed_int8_dot(const int8_t *a, const int8_t *b, int n) {
#if defined(__AVX512VNNI__) && defined(__AVX512BW__)
    return vnni_int8_dot(a, b, n);
#else
    return scalar_int8_dot(a, b, n);
#endif
}

/* ==========================================================================
 * TEST 1: Signed int8 dot-product kernel
 * ========================================================================== */
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

        int32_t got = signed_int8_dot(a, b, 16);
        int32_t expected = 16 * (int32_t)v8 * (int32_t)v8;

        printf("  v=%3d: dot(%d, %d) = %11d  expected=%11d  %s\n",
               v, v8, v8, got, expected, got == expected ? "OK" : "MISMATCH");
    }

    printf("\n");
}

/* ==========================================================================
 * TEST 2: Signed int8 data
 * ========================================================================== */
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
        int32_t got = signed_int8_dot(a, b, N);

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
        int32_t got = signed_int8_dot(a, b, N);

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
        int32_t got = signed_int8_dot(a, b, N);

        printf("  Case C (neg×neg):  ref=%d  got=%d  %s\n",
               ref, got, got == ref ? "OK" : "FAIL");
    }

    printf("\n");
}

/* ==========================================================================
 * TEST 3: Inspect intermediate values
 * ========================================================================== */
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

    int32_t dot = signed_int8_dot(a, b, 16);
    printf("  Signed int8 dot product: %d\n", dot);
    printf("  Expected sum of products: %d\n", 136);

    printf("\n");
}

/* ==========================================================================
 * TEST 4: Alternative approaches
 * ========================================================================== */
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

    printf("  %s dot product: %d\n", dot_kernel_name(), signed_int8_dot(a, b, N));
    printf("\n");
}

/* ==========================================================================
 * TEST 5: Larger matrix dimensions (simulating real matmul)
 * ========================================================================== */
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
    float y_opt[O];
    for (int o = 0; o < O; o++) {
        int32_t sum = scalar_int8_dot(x, w + (size_t)o * I, I);
        y_ref[o] = (float)sum * scales[o];
        y_opt[o] = (float)signed_int8_dot(x, w + (size_t)o * I, I) * scales[o];
    }

    for (int o = 0; o < O; o++) {
        float err = fabsf(y_opt[o] - y_ref[o]);
        printf("  row %d: ref=%.1f  opt=%.1f  err=%.1e  %s\n",
               o, y_ref[o], y_opt[o], err, err < 1e-3f ? "OK" : "FAIL");
    }

    printf("\n");
}

/* ==========================================================================
 * TEST 6: Performance sanity check
 * ========================================================================== */
static void test_benchmark(void) {
    printf("=== TEST 6: Performance sanity check ===\n\n");

    const int N = 4096;
    int8_t a[N];
    int8_t b[N];
    for (int i = 0; i < N; i++) {
        a[i] = (int8_t)(((i * 37 + 11) % 257) - 128);
        b[i] = (int8_t)(((i * 19 - 13) % 257) - 128);
    }

    int32_t ref = scalar_int8_dot(a, b, N);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int32_t got = signed_int8_dot(a, b, N);
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                        (end.tv_nsec - start.tv_nsec) / 1000000.0;

    printf("  kernel=%s  ref=%d  got=%d  elapsed=%.3f ms  %s\n",
           dot_kernel_name(), ref, got, elapsed_ms, ref == got ? "OK" : "FAIL");
    printf("\n");
}

/* ==========================================================================
 * MAIN
 * ========================================================================== */
int main(void) {
    printf("Signed int8 dot-product test for Strix Halo\n");
    printf("==========================================\n\n");

    test_intrinsic_behavior();
    test_sign_extended_int8();
    test_inspect_intermediates();
    test_alternative_approaches();
    test_larger_matrix();
    test_benchmark();

    printf("==========================================\n");
    printf("Tests complete.\n");

    return 0;
}
