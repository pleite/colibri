#ifndef COLIBRI_BACKEND_RUNTIME_H
#define COLIBRI_BACKEND_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COLI_RUNTIME_MAX_DEVICES 16

typedef struct ColiCudaTensor ColiCudaTensor;

/* Backend mask bits returned by coli_runtime_backend_mask(). */
#define COLI_RUNTIME_BACKEND_CPU_BIT    1
#define COLI_RUNTIME_BACKEND_NPU_BIT    2
#define COLI_RUNTIME_BACKEND_VULKAN_BIT 4
#define COLI_RUNTIME_BACKEND_CUDA_BIT   8

/* Operation roles ("sensors") let the caller describe the shape/behaviour of a
 * matmul so the scheduler can steer it toward the most suitable backend when
 * several are active simultaneously:
 *   - ATTENTION / SHARED_EXPERT are fixed-shape and always active, which the
 *     NPU (AIE2 tile ISA) handles best.
 *   - ROUTED_EXPERT / DENSE are variable-shape or large matrices, best on the
 *     GPU (RDNA/CUDA).
 *   - SMALL covers norms/routers/biases, cheapest to keep on the CPU.
 *   - AUTO keeps the historical even split across every active backend. */
typedef enum {
    COLI_OP_ROLE_AUTO = 0,
    COLI_OP_ROLE_ATTENTION = 1,
    COLI_OP_ROLE_SHARED_EXPERT = 2,
    COLI_OP_ROLE_ROUTED_EXPERT = 3,
    COLI_OP_ROLE_DENSE = 4,
    COLI_OP_ROLE_SMALL = 5,
    COLI_OP_ROLE_COUNT = 6
} coli_op_role;

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

/* Role-aware variant: identical to coli_runtime_matmul but steers the output
 * partition across the active backends according to the operation role. The
 * plain coli_runtime_matmul is COLI_OP_ROLE_AUTO. */
int coli_runtime_matmul_ex(ColiCudaTensor **tensor,
                           float *y, const float *x,
                           const void *weights, const float *scales,
                           int fmt, int S, int I, int O, int device,
                           coli_op_role role);

/* Reporting helpers used by the planner / doctor / server. */
int coli_runtime_backend_mask(void);
int coli_runtime_parallel_enabled(void);
/* Writes a comma-separated list of active backend names (e.g. "cpu,npu,cuda")
 * into buf; returns the number of active backends. */
int coli_runtime_active_backends(char *buf, size_t buflen);

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
