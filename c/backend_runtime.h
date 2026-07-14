#ifndef COLIBRI_BACKEND_RUNTIME_H
#define COLIBRI_BACKEND_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COLI_RUNTIME_MAX_DEVICES 16
#define COLI_CUDA_MAX_DEVICES COLI_RUNTIME_MAX_DEVICES

typedef struct ColiCudaTensor ColiCudaTensor;

int coli_runtime_init(const int *devices, int count);
void coli_runtime_shutdown(void);
int coli_runtime_device_count(void);
int coli_runtime_device_at(int index);
int coli_runtime_mem_info(int device, size_t *free_bytes, size_t *total_bytes);
void coli_runtime_stats(int device, size_t *tensor_count, size_t *tensor_bytes);
size_t coli_runtime_cache_hits(void);

int coli_runtime_tensor_upload(ColiCudaTensor **tensor,
                               const void *weights, const float *scales,
                               int fmt, int I, int O, int device);

int coli_runtime_matmul(ColiCudaTensor **tensor,
                        float *y, const float *x,
                        const void *weights, const float *scales,
                        int fmt, int S, int I, int O, int device);

void coli_runtime_tensor_free(ColiCudaTensor *tensor);
size_t coli_runtime_tensor_bytes(const ColiCudaTensor *tensor);
int coli_runtime_tensor_device(const ColiCudaTensor *tensor);

/* Compatibility entry points used by glm.c and qwen35_moe.c. */
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

#endif
