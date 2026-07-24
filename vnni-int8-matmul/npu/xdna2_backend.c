#include "xdna2_backend.h"

#include <stdint.h>
#include <stdio.h>

#include "../cpu/vnni_cpu_backend.h"

static int is_supported_shape(int rows, int out_cols) {
    return rows == 1 && out_cols == 4;
}

int strix_xdna2_matmul(const int8_t *input,
                       int rows,
                       int inner_dim,
                       const int8_t *weights,
                       int out_cols,
                       float *output,
                       const float *scales) {
    if (!is_supported_shape(rows, out_cols)) {
        fprintf(stderr, "xdna2 backend: fixed shape requires rows=1 and out_cols=4\n");
        return 0;
    }
    return strix_cpu_matmul(input, rows, inner_dim, weights, out_cols, output, scales);
}

const char *strix_xdna2_backend_name(void) {
    return "xdna2-fixed-4x1";
}
