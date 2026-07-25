#ifndef XDNA2_MATMUL_H
#define XDNA2_MATMUL_H

/**
 * xdna2_matmul.h — NPU matmul kernel interface for XDNA 2
 *
 * Fixed-shape int8 matrix multiplication for XDNA 2 AIE-2.
 * Each kernel shape requires a separately compiled AIE kernel binary.
 *
 * For shapes not covered by NPU kernels, falls back to CPU implementation.
 */

#include <stdint.h>

/* ── Kernel shape descriptor ── */

typedef struct {
    int32_t rows;        /* Batch size (typically 1 for inference) */
    int32_t inner_dim;   /* Input dimension (hidden_dim) */
    int32_t out_cols;    /* Output dimension */
    int32_t fmt;         /* Weight format: 0=F32, 1=INT8, 2=INT4 */
} xdna2_matmul_shape_t;

/* ── Kernel handle ── */

typedef struct {
    xdna2_matmul_shape_t shape;
    uint32_t kernel_handle;  /* NPU kernel handle */
    uint32_t num_args;       /* Number of kernel arguments */
    uint32_t *arg_sizes;     /* Size of each argument in bytes */
    int compiled;            /* Whether the kernel binary is loaded */
} xdna2_kernel_t;

/* ── NPU runtime state ── */

typedef struct {
    int device_fd;                     /* /dev/dri/card1 */
    int initialized;
    void *cpu_backend;                 /* CPU fallback function pointer */
} xdna2_runtime_t;

/* ═══════════════════════════════════════════════════════════
 * Kernel Compilation & Loading
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_compile_kernel — Compile an AIE kernel for a specific shape
 *
 * Requires aiecompiler (not included in base XRT).
 * Returns kernel handle on success, 0 on failure.
 *
 * In the current implementation, this is a stub that returns 0
 * because aiecompiler is not available. The CPU fallback is used.
 *
 * Future: when aiecompiler is installed, this will:
 *   1. Generate AIE C source for the matmul kernel
 *   2. Compile with aiecompiler to .aie package
 *   3. Package as .xclbin for XRT loading
 *   4. Return kernel handle for runtime execution
 */
uint32_t xdna2_compile_kernel(const xdna2_matmul_shape_t *shape,
                               const char *output_path);

/**
 * xdna2_load_kernel — Load a pre-compiled kernel binary
 *
 * Loads a .xclbin or .aie package file into the NPU.
 * Returns kernel handle on success, 0 on failure.
 */
uint32_t xdna2_load_kernel(const char *kernel_path,
                            const xdna2_matmul_shape_t *shape);

/**
 * xdna2_free_kernel — Release a kernel handle
 */
void xdna2_free_kernel(uint32_t kernel_handle);

/* ═══════════════════════════════════════════════════════════
 * Runtime Management
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_runtime_init — Initialize NPU runtime
 *
 * Opens device, creates hardware context, queries capabilities.
 * Returns 0 on success, negative on failure.
 */
int xdna2_runtime_init(xdna2_runtime_t *runtime);

/**
 * xdna2_runtime_shutdown — Shutdown NPU runtime
 */
void xdna2_runtime_shutdown(xdna2_runtime_t *runtime);

/* ═══════════════════════════════════════════════════════════
 * Matmul Execution
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_matmul_int8 — Execute int8 matrix multiplication on NPU
 *
 * y = X * W^T (where W is stored as [O x I] matrix)
 *
 * Args:
 *   x:      Input matrix [S x I] (float32)
 *   weights: Weight matrix [O x I] (int8, row-major)
 *   scales: Per-output-row scales [O] (float32)
 *   y:      Output matrix [S x O] (float32)
 *   S:      Batch size (sequence length)
 *   I:      Input dimension
 *   O:      Output dimension
 *
 * Returns 0 on success, negative on failure.
 * Falls back to CPU if NPU kernel not available for this shape.
 */
int xdna2_matmul_int8(const float *x, const int8_t *weights,
                       const float *scales, float *y,
                       int S, int I, int O,
                       xdna2_runtime_t *runtime);

/**
 * xdna2_matmul_int4 — Execute int4 matrix multiplication on NPU
 *
 * Dequantizes int4 to int8 on-the-fly, then uses int8 kernel.
 * Falls back to CPU for unsupported shapes.
 */
int xdna2_matmul_int4(const float *x, const uint8_t *weights_packed,
                       const float *scales, float *y,
                       int S, int I, int O,
                       xdna2_runtime_t *runtime);

/* ═══════════════════════════════════════════════════════════
 * Shape Registration
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_register_shape — Register a kernel shape for automatic selection
 *
 * When a matmul request comes in, the runtime checks if a compiled
 * kernel exists for that shape. If yes, uses NPU. If no, falls back
 * to CPU.
 */
int xdna2_register_shape(xdna2_runtime_t *runtime,
                          const xdna2_matmul_shape_t *shape,
                          uint32_t kernel_handle);

/**
 * xdna2_find_kernel — Find a compiled kernel for a given shape
 *
 * Returns kernel handle if found, 0 if not (caller should use CPU).
 */
uint32_t xdna2_find_kernel(xdna2_runtime_t *runtime,
                            int rows, int inner_dim, int out_cols);

#endif /* XDNA2_MATMUL_H */
