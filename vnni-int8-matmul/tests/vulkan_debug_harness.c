#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../gpu/vulkan_backend.h"

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

static void scalar_reference(const int8_t *input,
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
}

int main(void) {
    setenv("VNNI_VULKAN_DEBUG", "1", 0);

    const int rows = 2;
    const int inner_dim = 32;
    const int out_cols = 3;
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
    if (!strix_vulkan_is_supported()) {
        printf("Vulkan debug harness: backend unavailable (%s)\n",
               strix_vulkan_failure_reason());
        return 0;
    }
    printf("Vulkan debug harness: device '%s'\n", strix_vulkan_device_name());
    if (!strix_vulkan_matmul(input, rows, inner_dim, weights, out_cols, got, scales)) {
        fprintf(stderr, "Vulkan debug harness: backend execution failed\n");
        return 1;
    }

    printf("Vulkan debug harness using backend %s\n", strix_vulkan_backend_name());
    for (int i = 0; i < rows * out_cols; ++i) {
        printf("result[%d]=%.6f expected=%.6f\n", i, got[i], expected[i]);
    }
    printf("Vulkan debug harness: completed\n");
    strix_vulkan_shutdown();
    return 0;
}
