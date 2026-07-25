#ifndef XDNA2_MATMUL_H
#define XDNA2_MATMUL_H

/**
 * xdna2_matmul.h — fixed-shape INT8 matmul on the AMD XDNA 2 NPU (Strix Halo).
 *
 * Scope and non-goals
 * -------------------
 * This is a Strix Halo building block for inference. It is deliberately *not*
 * portable and it has *no* CPU fallback: when the NPU cannot execute a request
 * every entry point returns a negative error code so that the caller can decide
 * what to do. Silently computing on the CPU would hide missing kernels and make
 * NPU benchmarks meaningless.
 *
 * AIE-2 executes fixed-shape kernels only. A kernel binary is therefore bound
 * to one (rows, inner_dim, out_cols, fmt) tuple and must be produced ahead of
 * time by the AMD AIE toolchain. This runtime loads such an artifact, uploads
 * it to the NPU, and drives the DMA / submit / wait cycle around it.
 */

#include <stdint.h>
#include <stdbool.h>

#include "xdna2_driver.h"
#include "xdna2_xrt_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Weight formats ── */

#define XDNA2_FMT_INT8 1
#define XDNA2_FMT_INT4 2

/* ── Kernel shape descriptor ── */

typedef struct {
    int32_t rows;        /* Batch size (1 for single-token decode) */
    int32_t inner_dim;   /* Input dimension */
    int32_t out_cols;    /* Output dimension */
    int32_t fmt;         /* XDNA2_FMT_INT8 or XDNA2_FMT_INT4 */
} xdna2_matmul_shape_t;

/**
 * On-disk kernel artifact ("*.npukernel").
 *
 * The AIE toolchain (aiecompiler / xclbinutil) is the only thing that can
 * produce the DPU instruction stream for a given shape. Rather than guessing
 * at an encoding in C, this runtime consumes a small self-describing container
 * so the ERT opcode and CU mask travel with the instruction blob:
 *
 *   offset  size  field
 *   0       4     magic      'X','D','N','2' (0x324E4458 little-endian)
 *   4       4     version    format version, currently 1
 *   8       4     ert_opcode ERT packet opcode for this kernel
 *   12      4     cu_mask    compute-unit mask for the ERT packet
 *   16      4     rows
 *   20      4     inner_dim
 *   24      4     out_cols
 *   28      4     fmt
 *   32      4     instr_size instruction stream length in bytes
 *   36      4     instr_words number of 32-bit words the DPU should execute
 *   40      instr_size  DPU instruction stream
 *
 * All fields are little-endian, matching the host.
 */
#define XDNA2_KERNEL_MAGIC   0x324E4458u
#define XDNA2_KERNEL_VERSION 1u
#define XDNA2_KERNEL_HEADER_BYTES 40u

typedef struct {
    xdna2_matmul_shape_t shape;
    uint32_t   ert_opcode;
    uint32_t   cu_mask;
    uint32_t   instr_words;
    xdna2_bo_t instr_bo;   /* Instruction stream resident on the NPU */
    bool       loaded;
} xdna2_kernel_t;

/* ── Runtime state ── */

#define XDNA2_MAX_KERNELS 16

typedef struct {
    int             device_fd;
    xdna2_hwctx_t   hwctx;
    xdna2_kernel_t  kernels[XDNA2_MAX_KERNELS];
    int             kernel_count;
    uint32_t        timeout_ms;   /* Command wait timeout, default 5000 */
    /* Control plane the device was validated through (never AUTO). Dispatch
     * itself is always DRM ioctls; see xdna2_xrt_driver.h. */
    xdna2_control_plane_t control_plane;
    bool            initialized;
} xdna2_runtime_t;

/* ═══════════════════════════════════════════════════════════
 * Runtime management
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_runtime_init — open the NPU, create a hardware context.
 *
 * The control plane is chosen from XDNA2_DRIVER (auto, drm, xrt); see
 * xdna2_xrt_driver.h. Returns -EINVAL for an unknown value and -ENOSYS when
 * XDNA2_DRIVER=xrt but the XRT/XDNA shim stack is unusable.
 *
 * Returns 0 on success, negative on failure. On failure the runtime owns no
 * resources and must not be shut down.
 */
int xdna2_runtime_init(xdna2_runtime_t *runtime);

/**
 * xdna2_runtime_shutdown — release kernels, hardware context and device fd.
 * Safe to call on a zeroed or already-shut-down runtime.
 */
void xdna2_runtime_shutdown(xdna2_runtime_t *runtime);

/* ═══════════════════════════════════════════════════════════
 * Kernel loading
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_load_kernel — load a *.npukernel artifact and upload its instruction
 * stream to the NPU.
 *
 * Returns 0 on success, negative on failure.
 */
int xdna2_load_kernel(xdna2_runtime_t *runtime, const char *kernel_path);

/**
 * xdna2_find_kernel — look up a loaded kernel by exact shape.
 * Returns NULL when no kernel matches; the caller must not fall back silently.
 */
const xdna2_kernel_t *xdna2_find_kernel(const xdna2_runtime_t *runtime,
                                        int rows, int inner_dim, int out_cols,
                                        int fmt);

/* ═══════════════════════════════════════════════════════════
 * Execution
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_matmul_int8 — y[S x O] = x[S x I] * weights^T[O x I], per-column scaled.
 *
 * x:       int8 activations, row-major [S][I]
 * weights: int8 weights, row-major [O][I]
 * scales:  per-output-column f32 scales, or NULL
 * y:       f32 output, row-major [S][O]
 *
 * Returns 0 on success. Returns -ENOENT when no kernel is loaded for this exact
 * shape; there is no CPU fallback.
 */
int xdna2_matmul_int8(xdna2_runtime_t *runtime,
                      const int8_t *x, const int8_t *weights,
                      const float *scales, float *y,
                      int S, int I, int O);

/**
 * xdna2_dequant_int4 — expand packed int4 weights to int8, two nibbles per
 * byte, zero point 8. Output buffer must hold O*I bytes.
 *
 * The AIE-2 MAC datapath has no int4 mode, so int4 weights must be widened
 * before they can be fed to an int8 kernel.
 */
void xdna2_dequant_int4(const uint8_t *packed, int8_t *out, int O, int I);

#ifdef __cplusplus
}
#endif

#endif /* XDNA2_MATMUL_H */
