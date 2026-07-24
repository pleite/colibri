#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cpu/vnni_cpu_backend.h"
#include "../gpu/vulkan_backend.h"
#include "../npu/xdna2_backend.h"

static int scalar_reference(const int8_t *input,
                            int rows,
                            int inner_dim,
                            const int8_t *weights,
                            int out_cols,
                            float *output,
                            const float *scales) {
    for (int r = 0; r < rows; ++r) {
        for (int o = 0; o < out_cols; ++o) {
            int32_t acc = 0;
            for (int k = 0; k < inner_dim; ++k) {
                acc += (int32_t)input[(size_t)r * (size_t)inner_dim + k] *
                       (int32_t)weights[(size_t)o * (size_t)inner_dim + k];
            }
            output[(size_t)r * (size_t)out_cols + o] = (float)acc * (scales ? scales[o] : 1.0f);
        }
    }
    return 1;
}

static int compare_outputs(const float *got, const float *expected, int count, float tol) {
    for (int i = 0; i < count; ++i) {
        if (fabsf(got[i] - expected[i]) > tol) {
            fprintf(stderr, "mismatch at %d: got=%f expected=%f\n", i, got[i], expected[i]);
            return 0;
        }
    }
    return 1;
}

static void fill_input(int8_t *buffer, int count) {
    for (int i = 0; i < count; ++i) {
        buffer[i] = (int8_t)((i * 3 + 1) % 17 - 8);
    }
}

static void fill_weights(int8_t *buffer, int count) {
    for (int i = 0; i < count; ++i) {
        buffer[i] = (int8_t)((i * 5 + 3) % 13 - 6);
    }
}

static int run_cpu_case(void) {
    if (!strix_cpu_is_supported()) {
        printf("CPU backend SKIP (requires AVX-512 VNNI on Strix Halo)\n");
        return 1;
    }

    const int rows = 2;
    const int inner_dim = 32;
    const int out_cols = 4;
    int8_t input[(size_t)rows * (size_t)inner_dim];
    int8_t weights[(size_t)out_cols * (size_t)inner_dim];
    float scales[out_cols];
    float expected[(size_t)rows * (size_t)out_cols];
    float got[(size_t)rows * (size_t)out_cols];

    fill_input(input, rows * inner_dim);
    fill_weights(weights, out_cols * inner_dim);
    for (int i = 0; i < out_cols; ++i) {
        scales[i] = 0.5f + 0.125f * (float)i;
    }

    scalar_reference(input, rows, inner_dim, weights, out_cols, expected, scales);
    if (!strix_cpu_matmul(input, rows, inner_dim, weights, out_cols, got, scales)) {
        fprintf(stderr, "CPU backend returned failure\n");
        return 0;
    }
    if (!compare_outputs(got, expected, rows * out_cols, 1e-4f)) {
        fprintf(stderr, "CPU backend mismatch\n");
        return 0;
    }
    printf("CPU backend OK (%s)\n", strix_cpu_backend_name());
    return 1;
}

static int run_vulkan_case(void) {
    if (!strix_cpu_is_supported()) {
        printf("Vulkan backend SKIP (CPU backend unavailable on this host)\n");
        return 1;
    }

    const int rows = 1;
    const int inner_dim = 64;
    const int out_cols = 3;
    int8_t input[(size_t)rows * (size_t)inner_dim];
    int8_t weights[(size_t)out_cols * (size_t)inner_dim];
    float scales[out_cols];
    float expected[(size_t)rows * (size_t)out_cols];
    float got[(size_t)rows * (size_t)out_cols];

    fill_input(input, rows * inner_dim);
    fill_weights(weights, out_cols * inner_dim);
    for (int i = 0; i < out_cols; ++i) {
        scales[i] = 1.0f + 0.25f * (float)i;
    }

    scalar_reference(input, rows, inner_dim, weights, out_cols, expected, scales);
    if (!strix_vulkan_matmul(input, rows, inner_dim, weights, out_cols, got, scales)) {
        fprintf(stderr, "Vulkan backend returned failure\n");
        return 0;
    }
    if (!compare_outputs(got, expected, rows * out_cols, 1e-4f)) {
        fprintf(stderr, "Vulkan backend mismatch\n");
        return 0;
    }
    printf("Vulkan backend OK (%s)\n", strix_vulkan_backend_name());
    return 1;
}

static int run_xdna2_case(void) {
    if (!strix_cpu_is_supported()) {
        printf("XDNA2 backend SKIP (CPU backend unavailable on this host)\n");
        return 1;
    }

    const int rows = 1;
    const int inner_dim = 48;
    const int out_cols = 4;
    int8_t input[(size_t)rows * (size_t)inner_dim];
    int8_t weights[(size_t)out_cols * (size_t)inner_dim];
    float scales[out_cols];
    float expected[(size_t)rows * (size_t)out_cols];
    float got[(size_t)rows * (size_t)out_cols];

    fill_input(input, rows * inner_dim);
    fill_weights(weights, out_cols * inner_dim);
    for (int i = 0; i < out_cols; ++i) {
        scales[i] = 0.75f + 0.1f * (float)i;
    }

    scalar_reference(input, rows, inner_dim, weights, out_cols, expected, scales);
    if (!strix_xdna2_matmul(input, rows, inner_dim, weights, out_cols, got, scales)) {
        fprintf(stderr, "XDNA2 backend returned failure\n");
        return 0;
    }
    if (!compare_outputs(got, expected, rows * out_cols, 1e-4f)) {
        fprintf(stderr, "XDNA2 backend mismatch\n");
        return 0;
    }
    printf("XDNA2 backend OK (%s)\n", strix_xdna2_backend_name());
    return 1;
}

int main(void) {
    if (!run_cpu_case()) return 1;
    if (!run_vulkan_case()) return 1;
    if (!run_xdna2_case()) return 1;
    printf("All backend tests passed or skipped for a non-Strix-Halo host.\n");
    return 0;
}
