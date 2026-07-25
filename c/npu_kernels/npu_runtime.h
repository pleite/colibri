#ifndef NPU_RUNTIME_H
#define NPU_RUNTIME_H

/**
 * npu_runtime.h — NPU runtime integration header for colibri
 *
 * Provides the interface between colibri's backend_npu.c dispatcher
 * and the XDNA 2 NPU implementation in npu_kernels/.
 *
 * This header is included by backend_npu.c and provides the
 * coli_npu_* API that the colibri engine expects.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Backend capabilities ── */

#define COLI_NPU_MAX_DEVICES 4
#define COLI_CUDA_MAX_DEVICES 4

/* ── Tensor format ── */

#define COLI_FMT_F32  0
#define COLI_FMT_INT8 1
#define COLI_FMT_INT4 2
#define COLI_FMT_INT2 3

/* ── Tensor structure ── */

typedef struct ColiCudaTensor {
    int fmt;                  /* Weight format */
    int I;                    /* Input dimension */
    int O;                    /* Output dimension */
    int device;               /* NPU device index */
    void *weights;            /* Weight data (int8 or packed int4) */
    float *scales;            /* Per-output-row scales (F32) */
    size_t weight_bytes;      /* Size of weight data */
    size_t scale_bytes;       /* Size of scale data */
} ColiCudaTensor;

/* ═══════════════════════════════════════════════════════════
 * Backend API (matches backend_npu.c interface)
 * ═══════════════════════════════════════════════════════════ */

/**
 * coli_npu_init — Initialize NPU backend
 *
 * devices: array of device indices (NULL = use first available)
 * count: number of devices
 * Returns 1 on success, 0 on failure
 */
int coli_npu_init(const int *devices, int count);

/**
 * coli_npu_shutdown — Shutdown NPU backend
 */
void coli_npu_shutdown(void);

/**
 * coli_npu_device_count — Get number of available NPU devices
 */
int coli_npu_device_count(void);

/**
 * coli_npu_device_at — Get device index at position
 */
int coli_npu_device_at(int index);

/**
 * coli_npu_mem_info — Get memory info for a device
 */
int coli_npu_mem_info(int device, size_t *free_bytes, size_t *total_bytes);

/**
 * coli_npu_stats — Get backend statistics
 */
void coli_npu_stats(int device, size_t *tensor_count, size_t *tensor_bytes);

/**
 * coli_npu_tensor_upload — Upload weights to NPU
 *
 * tensor: output pointer (allocated by caller)
 * weights: weight data
 * scales: per-row scales
 * fmt: weight format (COLI_FMT_INT8, COLI_FMT_INT4, etc.)
 * I: input dimension
 * O: output dimension
 * device: NPU device index
 * Returns 1 on success, 0 on failure
 */
int coli_npu_tensor_upload(ColiCudaTensor **tensor,
                           const void *weights, const float *scales,
                           int fmt, int I, int O, int device);

/**
 * coli_npu_matmul — Execute matrix multiplication on NPU
 *
 * tensor: pre-uploaded tensor (or NULL for inline upload)
 * y: output matrix [S x O]
 * x: input matrix [S x I]
 * weights: weight data (if tensor is NULL)
 * scales: scales (if tensor is NULL)
 * fmt: weight format
 * S: batch size
 * I: input dimension
 * O: output dimension
 * device: NPU device index
 * Returns 1 on success, 0 on failure
 */
int coli_npu_matmul(ColiCudaTensor **tensor,
                    float *y, const float *x,
                    const void *weights, const float *scales,
                    int fmt, int S, int I, int O, int device);

/**
 * coli_npu_tensor_free — Free a tensor
 */
void coli_npu_tensor_free(ColiCudaTensor *tensor);

/**
 * coli_npu_tensor_bytes — Get total bytes for a tensor
 */
size_t coli_npu_tensor_bytes(const ColiCudaTensor *tensor);

/**
 * coli_npu_tensor_device — Get device index for a tensor
 */
int coli_npu_tensor_device(const ColiCudaTensor *tensor);

/* ── Aliases for backend_runtime.c compatibility ── */

int coli_cuda_init(const int *devices, int count);
void coli_cuda_shutdown(void);
int coli_cuda_device_count(void);
int coli_cuda_device_at(int index);
int coli_cuda_mem_info(int device, size_t *free_bytes, size_t *total_bytes);
void coli_cuda_stats(int device, size_t *tensor_count, size_t *tensor_bytes);
int coli_cuda_tensor_upload(ColiCudaTensor **tensor,
                             const void *weights, const float *scales,
                             int fmt, int I, int O, int device);
int coli_cuda_matmul(ColiCudaTensor **tensor,
                     float *y, const float *x,
                     const void *weights, const float *scales,
                     int fmt, int S, int I, int O, int device);
void coli_cuda_tensor_free(ColiCudaTensor *tensor);
size_t coli_cuda_tensor_bytes(const ColiCudaTensor *tensor);
int coli_cuda_tensor_device(const ColiCudaTensor *tensor);

#ifdef __cplusplus
}
#endif

#endif /* NPU_RUNTIME_H */

