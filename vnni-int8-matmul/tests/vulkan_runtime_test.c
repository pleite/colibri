#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../gpu/vulkan_backend.h"

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

int main(void) {
    const int rows = 2;
    const int inner_dim = 32;
    const int out_cols = 3;
    int8_t input[(size_t)rows * (size_t)inner_dim];
    int8_t weights[(size_t)out_cols * (size_t)inner_dim];
    float scales[out_cols];
    float expected[(size_t)rows * (size_t)out_cols];
    float got[(size_t)rows * (size_t)out_cols];

    for (int i = 0; i < rows * inner_dim; ++i) {
        input[i] = (int8_t)((i * 3 + 1) % 17 - 8);
    }
    for (int i = 0; i < out_cols * inner_dim; ++i) {
        weights[i] = (int8_t)((i * 5 + 3) % 13 - 6);
    }
    for (int i = 0; i < out_cols; ++i) {
        scales[i] = 0.5f + 0.125f * (float)i;
    }

    scalar_reference(input, rows, inner_dim, weights, out_cols, expected, scales);
    if (!strix_vulkan_matmul(input, rows, inner_dim, weights, out_cols, got, scales)) {
        fprintf(stderr, "vulkan backend returned failure\n");
        return 1;
    }
    if (!compare_outputs(got, expected, rows * out_cols, 1e-4f)) {
        fprintf(stderr, "vulkan backend mismatch\n");
        return 1;
    }

    float invalid_out[1] = {0.0f};
    if (strix_vulkan_matmul(NULL, rows, inner_dim, weights, out_cols, invalid_out, scales)) {
        fprintf(stderr, "invalid input should fail\n");
        return 1;
    }
    if (strix_vulkan_matmul(input, 0, inner_dim, weights, out_cols, invalid_out, scales)) {
        fprintf(stderr, "zero-row input should fail\n");
        return 1;
    }

    printf("Vulkan runtime test passed via %s\n", strix_vulkan_backend_name());
    return 0;
}
