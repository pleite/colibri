#ifndef VNNI_CPU_BACKEND_H
#define VNNI_CPU_BACKEND_H

#include <stddef.h>
#include <stdint.h>

int strix_cpu_is_supported(void);

int strix_cpu_matmul(const int8_t *input,
                     int rows,
                     int inner_dim,
                     const int8_t *weights,
                     int out_cols,
                     float *output,
                     const float *scales);

const char *strix_cpu_backend_name(void);

#endif
