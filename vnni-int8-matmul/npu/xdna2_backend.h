#ifndef XDNA2_BACKEND_H
#define XDNA2_BACKEND_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * XDNA 2 NPU backend, exclusive to AMD Strix Halo (Ryzen AI Max+ 395).
 *
 * This backend never computes on the CPU. AIE-2 executes fixed-shape kernels
 * only, so a kernel artifact must exist for the exact (rows, inner_dim,
 * out_cols) tuple being requested. When it does not, the call fails and
 * strix_xdna2_failure_reason() explains why.
 *
 * Kernel artifacts are discovered through the COLI_NPU_KERNEL_DIR environment
 * variable (default: ./npu/kernels) and are named
 *
 *     matmul_int8_<rows>x<inner_dim>x<out_cols>.npukernel
 */

/** Returns 1 when the NPU is open and a hardware context exists. */
int strix_xdna2_is_supported(void);

/**
 * output[rows][out_cols] = input[rows][inner_dim] * weights^T[out_cols][inner_dim]
 * scaled per output column. Returns 1 on success, 0 on failure.
 */
int strix_xdna2_matmul(const int8_t *input,
                       int rows,
                       int inner_dim,
                       const int8_t *weights,
                       int out_cols,
                       float *output,
                       const float *scales);

/** "xdna2-npu" when usable, "xdna2-unavailable" otherwise. */
const char *strix_xdna2_backend_name(void);

/** Human-readable reason the backend is unusable ("none" when usable). */
const char *strix_xdna2_failure_reason(void);

/** Releases the NPU hardware context. Safe to call when uninitialised. */
void strix_xdna2_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
