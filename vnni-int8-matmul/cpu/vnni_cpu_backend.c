#include "vnni_cpu_backend.h"

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

static int has_vnni_support(void) {
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx512vnni") && __builtin_cpu_supports("avx512bw");
}

int strix_cpu_is_supported(void) {
    return has_vnni_support();
}

static int32_t vnni_dot(const int8_t *a, const int8_t *b, int n) {
    __m512i acc = _mm512_setzero_si512();
    const __m512i zero = _mm512_setzero_si512();
    int i = 0;

    for (; i + 64 <= n; i += 64) {
        __m512i av = _mm512_loadu_si512((const void *)(a + i));
        __m512i bv = _mm512_loadu_si512((const void *)(b + i));
        __mmask64 neg = _mm512_movepi8_mask(bv);
        __m512i xs = _mm512_mask_sub_epi8(av, neg, zero, av);
        acc = _mm512_dpbusd_epi32(acc, _mm512_abs_epi8(bv), xs);
    }

    int32_t lanes[16];
    _mm512_storeu_si512((void *)lanes, acc);
    int32_t sum = 0;
    for (int lane = 0; lane < 16; ++lane) {
        sum += lanes[lane];
    }

    for (; i < n; ++i) {
        sum += (int32_t)a[i] * (int32_t)b[i];
    }
    return sum;
}

int strix_cpu_matmul(const int8_t *input,
                     int rows,
                     int inner_dim,
                     const int8_t *weights,
                     int out_cols,
                     float *output,
                     const float *scales) {
    if (!input || !weights || !output || rows <= 0 || inner_dim <= 0 || out_cols <= 0) {
        return 0;
    }
    if (!has_vnni_support()) {
        return 0;
    }

    for (int r = 0; r < rows; ++r) {
        const int8_t *row = input + (size_t)r * (size_t)inner_dim;
        for (int o = 0; o < out_cols; ++o) {
            const int8_t *wrow = weights + (size_t)o * (size_t)inner_dim;
            int32_t sum = vnni_dot(row, wrow, inner_dim);
            output[(size_t)r * (size_t)out_cols + o] = (float)sum * (scales ? scales[o] : 1.0f);
        }
    }
    return 1;
}

const char *strix_cpu_backend_name(void) {
    return has_vnni_support() ? "avx512-vnni" : "avx512-vnni-unavailable";
}
