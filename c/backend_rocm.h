#ifndef COLIBRI_BACKEND_ROCM_H
#define COLIBRI_BACKEND_ROCM_H

/*
 * ROCm/HIP backend for colibrì — same public API as backend_cuda.h so that
 * glm.c can include either header and use the same coli_cuda_* symbol names.
 *
 * Build with: make ROCM=1
 * Requires: ROCm ≥ 6.0 and hipcc (usually /opt/rocm/bin/hipcc).
 *
 * APU / unified memory (e.g. AMD Strix Halo — Radeon 890M):
 *   On integrated GPUs the GPU and CPU share the same physical DRAM.
 *   The backend detects this via hipDeviceAttributeIntegrated and uses
 *   hipHostMalloc(hipHostMallocMapped) for weight storage so that tensor
 *   uploads require only a CPU memcpy — no PCIe DMA.  The GPU accesses the
 *   host pointer directly via hipHostGetDevicePointer.
 *   Set COLI_ROCM_UNIFIED=1 to force this mode; =0 to disable it even on an
 *   integrated device.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COLI_CUDA_MAX_DEVICES 16

/* Opaque, persistent device copy of one resident quantized tensor. */
typedef struct ColiCudaTensor ColiCudaTensor;

int coli_cuda_init(const int *devices, int count);
void coli_cuda_shutdown(void);
int coli_cuda_device_count(void);
int coli_cuda_device_at(int index);
int coli_cuda_mem_info(int device, size_t *free_bytes, size_t *total_bytes);
void coli_cuda_stats(int device, size_t *tensor_count, size_t *tensor_bytes);

int coli_cuda_tensor_upload(ColiCudaTensor **tensor,
                            const void *weights, const float *scales,
                            int fmt, int I, int O, int device);

/*
 * y[S,O] = x[S,I] @ W[O,I]^T.
 * fmt: 0=f32, 1=int8, 2=int4, 3=int2.
 */
int coli_cuda_matmul(ColiCudaTensor **tensor,
                     float *y, const float *x,
                     const void *weights, const float *scales,
                     int fmt, int S, int I, int O, int device);

void coli_cuda_tensor_free(ColiCudaTensor *tensor);
size_t coli_cuda_tensor_bytes(const ColiCudaTensor *tensor);
int coli_cuda_tensor_device(const ColiCudaTensor *tensor);

int coli_rocm_init(const int *devices, int count);
void coli_rocm_shutdown(void);
int coli_rocm_device_count(void);
int coli_rocm_device_at(int index);
int coli_rocm_mem_info(int device, size_t *free_bytes, size_t *total_bytes);
void coli_rocm_stats(int device, size_t *tensor_count, size_t *tensor_bytes);

int coli_rocm_tensor_upload(ColiCudaTensor **tensor,
                            const void *weights, const float *scales,
                            int fmt, int I, int O, int device);

int coli_rocm_matmul(ColiCudaTensor **tensor,
                     float *y, const float *x,
                     const void *weights, const float *scales,
                     int fmt, int S, int I, int O, int device);

void coli_rocm_tensor_free(ColiCudaTensor *tensor);
size_t coli_rocm_tensor_bytes(const ColiCudaTensor *tensor);
int coli_rocm_tensor_device(const ColiCudaTensor *tensor);

#ifdef __cplusplus
}
#endif

#endif /* COLIBRI_BACKEND_ROCM_H */
