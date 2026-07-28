#ifndef XDNA2_BACKEND_H
#define XDNA2_BACKEND_H

#include <stddef.h>
#include <stdint.h>

/* For xdna2_matmul_timing_t. This header is pure stdint/stdbool; it does not
 * drag the amdxdna UAPI in. */
#include "c/npu_kernels/xdna2_matmul.h"

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
 * Returns 1 when a kernel artifact exists for exactly this shape, loading it
 * from COLI_NPU_KERNEL_DIR on first use, and 0 otherwise.
 *
 * This is the predicate the placement policy needs
 * (`coli_placement_caps_t::npu_kernel_exists`): an exact-shape artifact is a
 * hard precondition for choosing the NPU, and asking the backend is the only
 * way to answer it without a second copy of the artifact-naming rule.
 */
int strix_xdna2_kernel_exists(int rows, int inner_dim, int out_cols, int fmt);

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

/**
 * As strix_xdna2_matmul(), additionally reporting the per-stage cost of the
 * dispatch. `timing` may be NULL. Used by bench/backend_bench.c, which has to
 * separate the NPU's fixed cost from array time: at rows=1 the fixed cost is
 * the whole story and a single total would hide it.
 */
int strix_xdna2_matmul_timed(const int8_t *input,
                             int rows,
                             int inner_dim,
                             const int8_t *weights,
                             int out_cols,
                             float *output,
                             const float *scales,
                             xdna2_matmul_timing_t *timing);

/** "xdna2-npu" when usable, "xdna2-unavailable" otherwise. */
const char *strix_xdna2_backend_name(void);

/**
 * strix_xdna2_batch_matmul — dispatch `batch_size` matmuls in parallel.
 *
 * All batch items compute the same shape (rows, inner_dim, out_cols) with the
 * same `input` and `weights`.  `output` must point to a contiguous buffer of
 * at least `batch_size * rows * out_cols` f32 values; slice `b` starts at
 * `output + b * rows * out_cols`.
 *
 * On Strix Halo unified memory the input and weight BOs are allocated and
 * uploaded once; each batch item gets its own output BO.  All commands are
 * submitted without an intervening wait and a single timeline-syncobj wait
 * covers the whole batch.
 *
 * Returns 1 on success, 0 on failure.
 */
int strix_xdna2_batch_matmul(const int8_t *input,
                             int rows,
                             int inner_dim,
                             const int8_t *weights,
                             int out_cols,
                             float *output,
                             const float *scales,
                             int batch_size);

/**
 * Bytes the NPU can hold resident for one dispatch, or 0 when unknown.
 *
 * This is the size of the client device heap the runtime actually mapped —
 * every AMDXDNA_BO_DEV allocation is carved out of it, and the kernel caps it
 * at XDNA2_DEV_HEAP_MAX_BYTES (64 MiB). It is the number that belongs in
 * `coli_placement_caps_t::npu_resident_bytes`: an operand set larger than this
 * cannot be made resident, whatever the shape says.
 *
 * Returns 0 rather than a guess when the NPU is unavailable, which the
 * placement policy reads as "do not use residency as a constraint".
 */
size_t strix_xdna2_resident_bytes(void);

/** Human-readable reason the backend is unusable ("none" when usable). */
const char *strix_xdna2_failure_reason(void);

/** Releases the NPU hardware context. Safe to call when uninitialised. */
void strix_xdna2_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
