#ifndef XDNA2_BACKEND_H
#define XDNA2_BACKEND_H

#include <stdint.h>

int strix_xdna2_matmul(const int8_t *input,
                       int rows,
                       int inner_dim,
                       const int8_t *weights,
                       int out_cols,
                       float *output,
                       const float *scales);

const char *strix_xdna2_backend_name(void);

#endif
