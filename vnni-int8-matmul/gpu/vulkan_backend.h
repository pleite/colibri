#ifndef VULKAN_BACKEND_H
#define VULKAN_BACKEND_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Headless Vulkan compute backend, exclusive to the AMD Strix Halo iGPU
 * (Ryzen AI Max+ 395, RADV GFX1151). There is no fallback path: if a Strix Halo
 * iGPU with a compute queue is not present, every entry point below reports
 * failure rather than silently computing on another device.
 */

/** Returns 1 if a Strix Halo compute context is available, 0 otherwise. */
int strix_vulkan_is_supported(void);

/**
 * C[rows][out_cols] = A[rows][inner_dim] * B^T[out_cols][inner_dim], scaled
 * per output column by `scales` (may be NULL).
 *
 * Returns 1 on success, 0 on failure. Never falls back to the CPU.
 */
int strix_vulkan_matmul(const int8_t *input,
                        int rows,
                        int inner_dim,
                        const int8_t *weights,
                        int out_cols,
                        float *output,
                        const float *scales);

/**
 * Batched variant: records `batch_size` independent command buffers for the
 * same (input, weights) operands, submits them all with a single
 * vkQueueSubmit(), and waits for one fence.
 *
 * `output` must point to a contiguous buffer of at least
 * `batch_size * rows * out_cols` f32 values; slice `b` starts at
 * `output + b * rows * out_cols`.
 *
 * Because Strix Halo has unified memory, no staging copy is needed: the
 * HOST_VISIBLE|DEVICE_LOCAL path is used throughout, and the GPU reads
 * from the same physical pages the CPU wrote.
 *
 * Returns 1 on success, 0 on failure. Never falls back to the CPU.
 */
int strix_vulkan_batch_matmul(const int8_t *input,
                              int rows,
                              int inner_dim,
                              const int8_t *weights,
                              int out_cols,
                              float *output,
                              const float *scales,
                              int batch_size);

/** "vulkan-compute-strix-halo" when usable, "vulkan-unavailable" otherwise. */
const char *strix_vulkan_backend_name(void);

/**
 * Bytes the iGPU can hold resident for one dispatch, from the largest
 * DEVICE_LOCAL memory heap, or 0 when the context is unavailable.
 *
 * The number belongs in `coli_placement_caps_t::gpu_resident_bytes`. On this
 * unified-memory part the DEVICE_LOCAL heap is system memory, so it is a real
 * bound rather than a discrete-GPU VRAM figure; 0 means "unknown", which the
 * placement policy reads as "do not use residency as a constraint".
 */
size_t strix_vulkan_resident_bytes(void);

/** VkPhysicalDeviceProperties::deviceName of the selected iGPU, or "". */
const char *strix_vulkan_device_name(void);

/** Human-readable reason the backend is unavailable ("none" when usable). */
const char *strix_vulkan_failure_reason(void);

/** Tears down the process-wide context. Safe to call when uninitialised. */
void strix_vulkan_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
