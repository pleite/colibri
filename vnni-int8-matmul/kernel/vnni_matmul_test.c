#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../cpu/vnni_cpu_backend.h"

static void fill_input(int8_t *dst, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (int8_t)((i * 7 + 3) % 29 - 14);
    }
}

static void fill_weights(int8_t *dst, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (int8_t)((i * 5 + 1) % 23 - 11);
    }
}

int main(void) {
    const int rows = 2;
    const int inner_dim = 96;
    const int out_cols = 4;
    int8_t input[(size_t)rows * (size_t)inner_dim];
    int8_t weights[(size_t)out_cols * (size_t)inner_dim];
    float scales[out_cols];
    float output[(size_t)rows * (size_t)out_cols];

    fill_input(input, rows * inner_dim);
    fill_weights(weights, out_cols * inner_dim);
    for (int i = 0; i < out_cols; ++i) {
        scales[i] = 0.25f + 0.125f * (float)i;
    }

    if (!strix_cpu_matmul(input, rows, inner_dim, weights, out_cols, output, scales)) {
        fprintf(stderr, "CPU matmul failed\n");
        return 1;
    }

    printf("%s matmul demo\n", strix_cpu_backend_name());
    for (int r = 0; r < rows; ++r) {
        printf("row %d:", r);
        for (int o = 0; o < out_cols; ++o) {
            printf(" %0.3f", output[(size_t)r * (size_t)out_cols + o]);
        }
        putchar('\n');
    }
    return 0;
}
