#include "backend_runtime.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parallelism.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef COLI_ENABLE_NPU
#include "backend_npu.h"
#endif
#ifdef COLI_ENABLE_VULKAN
#include "backend_vulkan.h"
#endif
#if defined(COLI_CUDA) && !defined(COLI_ENABLE_VULKAN) && !defined(COLI_ROCM)
#include "backend_cuda.h"
#endif
#if defined(COLI_ROCM)
#include "backend_rocm.h"
#endif

struct ColiCudaTensor {
    int fmt;
    int I;
    int O;
    int device;
    void *weights;
    float *scales;
    size_t weight_bytes;
    size_t scale_bytes;
    float *cache_y;
    int cache_S;
    int cache_I;
    int cache_O;
    int cache_fmt;
    uint64_t cache_hash;
};

static const size_t k_stub_memory_bytes = 4ULL * 1024ULL * 1024ULL * 1024ULL;

static int g_initialized = 0;
static int g_devices[COLI_RUNTIME_MAX_DEVICES];
static int g_device_count = 0;
static size_t g_tensor_count = 0;
static size_t g_tensor_bytes = 0;
static ColiCudaTensor **g_tensors = NULL;
static size_t g_tensor_capacity = 0;
static size_t g_cache_hits = 0;
static int g_backend_mask = 0;

static int backend_bit_for_name(const char *name) {
    if (!name || !*name) return 0;
    if (!strcmp(name, "cpu")) return 1;
#ifdef COLI_ENABLE_NPU
    if (!strcmp(name, "npu")) return 2;
#endif
#ifdef COLI_ENABLE_VULKAN
    if (!strcmp(name, "vulkan")) return 4;
#endif
#if defined(COLI_CUDA)
    /* CUDA and ROCm share the same runtime dispatch slot because the build-time
     * mapping selects the appropriate implementation via the same coli_cuda_*
     * ABI entry points. */
    if (!strcmp(name, "cuda")) return 8;
#endif
#if defined(COLI_ROCM)
    if (!strcmp(name, "rocm")) return 8;
#endif
    return 0;
}

static int parse_backend_names(const char *value, int *mask_out) {
    int mask = 0;
    int saw_any = 0;
    if (!value || !*value) {
        *mask_out = 0;
        return 0;
    }
    char *copy = (char *)malloc(strlen(value) + 1);
    if (!copy) {
        fprintf(stderr, "runtime: memory allocation failed while parsing backend list '%s'\n", value);
        *mask_out = 0;
        return 0;
    }
    strcpy(copy, value);
    char *cursor = copy;
    while (cursor && *cursor) {
        char *comma = strchr(cursor, ',');
        if (comma) {
            *comma = '\0';
        }
        char *entry = cursor;
        while (*entry == ' ' || *entry == '\t') ++entry;
        char *end = entry + strlen(entry);
        while (end > entry && (end[-1] == ' ' || end[-1] == '\t')) {
            *--end = '\0';
        }
        if (*entry) {
            saw_any = 1;
            if (!strcmp(entry, "none")) {
                mask = 0;
            } else {
                int bit = backend_bit_for_name(entry);
                if (bit) mask |= bit;
            }
        }
        if (!comma) break;
        cursor = comma + 1;
    }
    free(copy);
    *mask_out = mask;
    return saw_any;
}

static int resolve_backend_mask(void) {
    int mask = 1;
#ifdef COLI_ENABLE_NPU
    mask |= 2;
#endif
#ifdef COLI_ENABLE_VULKAN
    mask |= 4;
#endif
#if defined(COLI_CUDA)
    mask |= 8;
#endif
#if defined(COLI_ROCM)
    mask |= 8;
#endif

    const char *disabled = getenv("COLI_RUNTIME_DISABLE_ENGINES");
    if (disabled && *disabled) {
        int disable_mask = 0;
        parse_backend_names(disabled, &disable_mask);
        mask &= ~disable_mask;
    }

    const char *enabled = getenv("COLI_RUNTIME_ENGINES");
    if (enabled && *enabled) {
        int requested_mask = 0;
        if (parse_backend_names(enabled, &requested_mask)) {
            mask = requested_mask;
        }
    }

    return mask > 0 ? mask : 1;
}

static void configure_parallelism(void) {
#ifdef _OPENMP
    int threads = coli_detect_thread_count();
    omp_set_dynamic(0);
    omp_set_num_threads(threads);
#endif
}

static int valid_fmt(int fmt) {
    return fmt == 0 || fmt == 1 || fmt == 2 || fmt == 3;
}

static int tensor_params_match(const ColiCudaTensor *tensor, int fmt, int I, int O, int device) {
    return tensor && tensor->fmt == fmt && tensor->I == I && tensor->O == O && tensor->device == device;
}

static size_t packed_bytes(int fmt, int I, int O) {
    if (fmt == 0) return (size_t)O * (size_t)I * sizeof(float);
    if (fmt == 1) return (size_t)O * (size_t)I * sizeof(int8_t);
    if (fmt == 2) return (size_t)O * (size_t)((I + 1) / 2) * sizeof(uint8_t);
    return (size_t)O * (size_t)((I + 3) / 4) * sizeof(uint8_t);
}

static size_t scale_bytes(int fmt, int O) {
    return (fmt == 0) ? 0 : (size_t)O * sizeof(float);
}

static void register_tensor(ColiCudaTensor *tensor) {
    if (!tensor) return;
    if (g_tensor_count == g_tensor_capacity) {
        size_t next_capacity = g_tensor_capacity ? g_tensor_capacity * 2 : 8;
        ColiCudaTensor **next = (ColiCudaTensor **)realloc(g_tensors, next_capacity * sizeof(*next));
        if (!next) return;
        g_tensors = next;
        g_tensor_capacity = next_capacity;
    }
    g_tensors[g_tensor_count++] = tensor;
    g_tensor_bytes += tensor->weight_bytes + tensor->scale_bytes;
}

static void unregister_tensor(ColiCudaTensor *tensor) {
    if (!tensor || !g_tensors) return;
    for (size_t i = 0; i < g_tensor_count; ++i) {
        if (g_tensors[i] == tensor) {
            if (i + 1 < g_tensor_count) {
                g_tensors[i] = g_tensors[g_tensor_count - 1];
            }
            g_tensor_count -= 1;
            g_tensor_bytes -= tensor->weight_bytes + tensor->scale_bytes;
            return;
        }
    }
}

static uint64_t fnv1a(const void *data, size_t len) {
    const unsigned char *p = (const unsigned char *)data;
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; ++i) {
        hash ^= p[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static void host_matmul(float *y, const float *x, const void *weights, const float *scales, int fmt, int S, int I, int O, int start, int count) {
    const float *weights_f32 = (const float *)weights;
    const int8_t *weights_i8 = (const int8_t *)weights;
    const uint8_t *weights_packed = (const uint8_t *)weights;
    const float *scale = scales;
    const int base = start;
    const int end = start + count;
    for (int o = base; o < end; ++o) {
        const float row_scale = (scale && fmt != 0) ? scale[o] : 1.0f;
        for (int s = 0; s < S; ++s) {
            const float *row = x + (size_t)s * (size_t)I;
            float acc = 0.0f;
            for (int i = 0; i < I; ++i) {
                if (fmt == 0) {
                    acc += row[i] * weights_f32[(size_t)o * (size_t)I + i];
                } else if (fmt == 1) {
                    acc += row[i] * (float)weights_i8[(size_t)o * (size_t)I + i];
                } else if (fmt == 2) {
                    const int idx = i;
                    const size_t packed_stride = (size_t)((I + 1) / 2);
                    const size_t packed_index = (size_t)o * packed_stride + (size_t)(idx >> 1);
                    const int value = (idx & 1) ? (weights_packed[packed_index] >> 4) & 0x0f : weights_packed[packed_index] & 0x0f;
                    acc += row[i] * (float)(value - 8);
                } else {
                    const int idx = i;
                    const size_t packed_stride = (size_t)((I + 3) / 4);
                    const size_t packed_index = (size_t)o * packed_stride + (size_t)(idx >> 2);
                    const int value = (weights_packed[packed_index] >> ((idx & 3) * 2)) & 0x03;
                    acc += row[i] * (float)(value - 2);
                }
            }
            y[(size_t)s * (size_t)O + (o - base)] = acc * row_scale;
        }
    }
}

static void *slice_weights(const void *weights, int fmt, int I, int O, int start, int count, size_t *out_bytes) {
    if (count <= 0 || start < 0 || start + count > O) return NULL;
    if (fmt == 0) {
        *out_bytes = (size_t)count * (size_t)I * sizeof(float);
        float *out = (float *)malloc(*out_bytes);
        if (!out) return NULL;
        memcpy(out, (const float *)weights + (size_t)start * (size_t)I, *out_bytes);
        return out;
    }
    if (fmt == 1) {
        *out_bytes = (size_t)count * (size_t)I * sizeof(int8_t);
        int8_t *out = (int8_t *)malloc(*out_bytes);
        if (!out) return NULL;
        memcpy(out, (const int8_t *)weights + (size_t)start * (size_t)I, *out_bytes);
        return out;
    }
    if (fmt == 2) {
        const size_t packed_per_row = (size_t)((I + 1) / 2);
        *out_bytes = (size_t)count * packed_per_row * sizeof(uint8_t);
        uint8_t *out = (uint8_t *)malloc(*out_bytes);
        if (!out) return NULL;
        const uint8_t *src = (const uint8_t *)weights;
        for (int row = 0; row < count; ++row) {
            memcpy(out + (size_t)row * packed_per_row, src + (size_t)(start + row) * packed_per_row, packed_per_row);
        }
        return out;
    }
    const size_t packed_per_row = (size_t)((I + 3) / 4);
    *out_bytes = (size_t)count * packed_per_row * sizeof(uint8_t);
    uint8_t *out = (uint8_t *)malloc(*out_bytes);
    if (!out) return NULL;
    const uint8_t *src = (const uint8_t *)weights;
    for (int row = 0; row < count; ++row) {
        memcpy(out + (size_t)row * packed_per_row, src + (size_t)(start + row) * packed_per_row, packed_per_row);
    }
    return out;
}

static float *slice_scales(const float *scales, int start, int count, size_t *out_bytes) {
    if (count <= 0) return NULL;
    *out_bytes = (size_t)count * sizeof(float);
    float *out = (float *)malloc(*out_bytes);
    if (!out) return NULL;
    memcpy(out, scales + start, *out_bytes);
    return out;
}

static int dispatch_chunk(float *y, const float *x, const void *weights, const float *scales, int fmt, int S, int I, int O, int backend_id, int start, int output_chunk_size, int device) {
    if (output_chunk_size <= 0) return 1;
    size_t weights_bytes = 0;
    size_t scale_bytes = 0;
    void *slice_weights_buf = slice_weights(weights, fmt, I, O, start, output_chunk_size, &weights_bytes);
    float *slice_scales_buf = NULL;
    if (fmt != 0 && scales) {
        slice_scales_buf = slice_scales(scales, start, output_chunk_size, &scale_bytes);
    }
    if (!slice_weights_buf || (fmt != 0 && scales && !slice_scales_buf)) {
        free(slice_weights_buf);
        free(slice_scales_buf);
        return 0;
    }

    if (backend_id == 0) {
        float *chunk_out = (float *)calloc((size_t)S * (size_t)output_chunk_size, sizeof(float));
        if (!chunk_out) {
            free(slice_weights_buf);
            free(slice_scales_buf);
            return 0;
        }
        host_matmul(chunk_out, x, slice_weights_buf, slice_scales_buf, fmt, S, I, output_chunk_size, 0, output_chunk_size);
        for (int s = 0; s < S; ++s) {
            for (int o = 0; o < output_chunk_size; ++o) {
                y[(size_t)s * (size_t)O + (size_t)(start + o)] = chunk_out[(size_t)s * (size_t)output_chunk_size + o];
            }
        }
        free(chunk_out);
        free(slice_weights_buf);
        free(slice_scales_buf);
        return 1;
    }

    if (backend_id == 1) {
#ifdef COLI_ENABLE_NPU
        ColiCudaTensor *tensor = NULL;
        if (!coli_npu_tensor_upload(&tensor, slice_weights_buf, slice_scales_buf, fmt, I, output_chunk_size, device)) {
            free(slice_weights_buf);
            free(slice_scales_buf);
            return 0;
        }
        float *chunk_out = (float *)calloc((size_t)S * (size_t)output_chunk_size, sizeof(float));
        if (!chunk_out) {
            coli_npu_tensor_free(tensor);
            free(slice_weights_buf);
            free(slice_scales_buf);
            return 0;
        }
        const int ok = coli_npu_matmul(&tensor, chunk_out, x, slice_weights_buf, slice_scales_buf, fmt, S, I, output_chunk_size, device);
        if (ok) {
            for (int s = 0; s < S; ++s) {
                for (int o = 0; o < output_chunk_size; ++o) y[(size_t)s * (size_t)O + (size_t)(start + o)] = chunk_out[(size_t)s * (size_t)output_chunk_size + o];
            }
        }
        free(chunk_out);
        coli_npu_tensor_free(tensor);
        free(slice_weights_buf);
        free(slice_scales_buf);
        return ok;
#else
        free(slice_weights_buf);
        free(slice_scales_buf);
        return 0;
#endif
    }

    if (backend_id == 2) {
#ifdef COLI_ENABLE_VULKAN
        ColiCudaTensor *tensor = NULL;
        if (!coli_vulkan_tensor_upload(&tensor, slice_weights_buf, slice_scales_buf, fmt, I, output_chunk_size, device)) {
            free(slice_weights_buf);
            free(slice_scales_buf);
            return 0;
        }
        float *chunk_out = (float *)calloc((size_t)S * (size_t)output_chunk_size, sizeof(float));
        if (!chunk_out) {
            coli_vulkan_tensor_free(tensor);
            free(slice_weights_buf);
            free(slice_scales_buf);
            return 0;
        }
        const int ok = coli_vulkan_matmul(&tensor, chunk_out, x, slice_weights_buf, slice_scales_buf, fmt, S, I, output_chunk_size, device);
        if (ok) {
            for (int s = 0; s < S; ++s) {
                for (int o = 0; o < output_chunk_size; ++o) y[(size_t)s * (size_t)O + (size_t)(start + o)] = chunk_out[(size_t)s * (size_t)output_chunk_size + o];
            }
        }
        free(chunk_out);
        coli_vulkan_tensor_free(tensor);
        free(slice_weights_buf);
        free(slice_scales_buf);
        return ok;
#else
        free(slice_weights_buf);
        free(slice_scales_buf);
        return 0;
#endif
    }

    if (backend_id == 3) {
#if defined(COLI_ROCM)
        ColiCudaTensor *tensor = NULL;
        if (!coli_rocm_tensor_upload(&tensor, slice_weights_buf, slice_scales_buf, fmt, I, output_chunk_size, device)) {
            free(slice_weights_buf);
            free(slice_scales_buf);
            return 0;
        }
        float *chunk_out = (float *)calloc((size_t)S * (size_t)output_chunk_size, sizeof(float));
        if (!chunk_out) {
            coli_rocm_tensor_free(tensor);
            free(slice_weights_buf);
            free(slice_scales_buf);
            return 0;
        }
        const int ok = coli_rocm_matmul(&tensor, chunk_out, x, slice_weights_buf, slice_scales_buf, fmt, S, I, output_chunk_size, device);
        if (ok) {
            for (int s = 0; s < S; ++s) {
                for (int o = 0; o < output_chunk_size; ++o) y[(size_t)s * (size_t)O + (size_t)(start + o)] = chunk_out[(size_t)s * (size_t)output_chunk_size + o];
            }
        }
        free(chunk_out);
        coli_rocm_tensor_free(tensor);
        free(slice_weights_buf);
        free(slice_scales_buf);
        return ok;
#elif defined(COLI_CUDA) && !defined(COLI_ENABLE_VULKAN)
        ColiCudaTensor *tensor = NULL;
        if (!coli_cuda_tensor_upload(&tensor, slice_weights_buf, slice_scales_buf, fmt, I, output_chunk_size, device)) {
            free(slice_weights_buf);
            free(slice_scales_buf);
            return 0;
        }
        float *chunk_out = (float *)calloc((size_t)S * (size_t)output_chunk_size, sizeof(float));
        if (!chunk_out) {
            coli_cuda_tensor_free(tensor);
            free(slice_weights_buf);
            free(slice_scales_buf);
            return 0;
        }
        const int ok = coli_cuda_matmul(&tensor, chunk_out, x, slice_weights_buf, slice_scales_buf, fmt, S, I, output_chunk_size, device);
        if (ok) {
            for (int s = 0; s < S; ++s) {
                for (int o = 0; o < output_chunk_size; ++o) y[(size_t)s * (size_t)O + (size_t)(start + o)] = chunk_out[(size_t)s * (size_t)output_chunk_size + o];
            }
        }
        free(chunk_out);
        coli_cuda_tensor_free(tensor);
        free(slice_weights_buf);
        free(slice_scales_buf);
        return ok;
#else
        free(slice_weights_buf);
        free(slice_scales_buf);
        return 0;
#endif
    }

    free(slice_weights_buf);
    free(slice_scales_buf);
    return 0;
}

int coli_runtime_init(const int *devices, int count) {
    if (g_initialized) return 1;
    if (!devices || count <= 0) {
        g_devices[0] = 0;
        g_device_count = 1;
    } else {
        g_device_count = count > COLI_RUNTIME_MAX_DEVICES ? COLI_RUNTIME_MAX_DEVICES : count;
        for (int i = 0; i < g_device_count; ++i) g_devices[i] = devices[i];
    }
    g_backend_mask = resolve_backend_mask();
    configure_parallelism();
#ifdef COLI_ENABLE_NPU
    if ((g_backend_mask & 2) && coli_npu_init(g_devices, g_device_count)) {
        g_backend_mask |= 2;
    } else {
        g_backend_mask &= ~2;
    }
#endif
#ifdef COLI_ENABLE_VULKAN
    if ((g_backend_mask & 4) && coli_vulkan_init(g_devices, g_device_count)) {
        g_backend_mask |= 4;
    } else {
        g_backend_mask &= ~4;
    }
#endif
#if defined(COLI_CUDA) && !defined(COLI_ENABLE_VULKAN) && !defined(COLI_ROCM)
    if ((g_backend_mask & 8) && coli_cuda_init(g_devices, g_device_count)) {
        g_backend_mask |= 8;
    } else {
        g_backend_mask &= ~8;
    }
#endif
#if defined(COLI_ROCM)
    if ((g_backend_mask & 8) && coli_rocm_init(g_devices, g_device_count)) {
        g_backend_mask |= 8;
    } else {
        g_backend_mask &= ~8;
    }
#endif
    g_initialized = 1;
    return 1;
}

void coli_runtime_shutdown(void) {
    while (g_tensor_count > 0) {
        coli_runtime_tensor_free(g_tensors[0]);
    }
    free(g_tensors);
    g_tensors = NULL;
    g_tensor_capacity = 0;
    g_initialized = 0;
    g_device_count = 0;
    g_tensor_count = 0;
    g_tensor_bytes = 0;
    g_cache_hits = 0;
#ifdef COLI_ENABLE_NPU
    coli_npu_shutdown();
#endif
#ifdef COLI_ENABLE_VULKAN
    coli_vulkan_shutdown();
#endif
#if defined(COLI_CUDA) && !defined(COLI_ENABLE_VULKAN) && !defined(COLI_ROCM)
    coli_cuda_shutdown();
#endif
#if defined(COLI_ROCM)
    coli_rocm_shutdown();
#endif
}

int coli_runtime_device_count(void) {
    return g_initialized ? g_device_count : 0;
}

int coli_runtime_device_at(int index) {
    if (!g_initialized || index < 0 || index >= g_device_count) return -1;
    return g_devices[index];
}

int coli_runtime_mem_info(int device, size_t *free_bytes, size_t *total_bytes) {
    (void)device;
    if (free_bytes) *free_bytes = k_stub_memory_bytes;
    if (total_bytes) *total_bytes = k_stub_memory_bytes;
    return 1;
}

void coli_runtime_stats(int device, size_t *tensor_count, size_t *tensor_bytes) {
    (void)device;
    if (tensor_count) *tensor_count = g_tensor_count;
    if (tensor_bytes) *tensor_bytes = g_tensor_bytes;
}

size_t coli_runtime_cache_hits(void) {
    return g_cache_hits;
}

int coli_runtime_tensor_upload(ColiCudaTensor **tensor,
                               const void *weights, const float *scales,
                               int fmt, int I, int O, int device) {
    if (!tensor || !valid_fmt(fmt) || I <= 0 || O <= 0 || device < 0) return 0;
    const int has_valid_payload = weights != NULL && (fmt == 0 || scales != NULL);
    if (!has_valid_payload) return 0;
    if (!g_initialized && !coli_runtime_init(NULL, 0)) return 0;
    if (*tensor) {
        ColiCudaTensor *existing = *tensor;
        if (tensor_params_match(existing, fmt, I, O, device)) {
            return 1;
        }
        return 0;
    }
    *tensor = (ColiCudaTensor *)calloc(1, sizeof(**tensor));
    if (!*tensor) return 0;

    (*tensor)->fmt = fmt;
    (*tensor)->I = I;
    (*tensor)->O = O;
    (*tensor)->device = device;
    (*tensor)->weight_bytes = packed_bytes(fmt, I, O);
    (*tensor)->scale_bytes = scale_bytes(fmt, O);
    if ((*tensor)->weight_bytes > 0) {
        (*tensor)->weights = malloc((*tensor)->weight_bytes);
        if (!(*tensor)->weights) {
            coli_runtime_tensor_free(*tensor);
            *tensor = NULL;
            return 0;
        }
        memcpy((*tensor)->weights, weights, (*tensor)->weight_bytes);
    }
    if ((*tensor)->scale_bytes > 0) {
        (*tensor)->scales = (float *)malloc((*tensor)->scale_bytes);
        if (!(*tensor)->scales) {
            coli_runtime_tensor_free(*tensor);
            *tensor = NULL;
            return 0;
        }
        memcpy((*tensor)->scales, scales, (*tensor)->scale_bytes);
    }

    register_tensor(*tensor);
    return 1;
}

int coli_runtime_matmul(ColiCudaTensor **tensor,
                        float *y, const float *x,
                        const void *weights, const float *scales,
                        int fmt, int S, int I, int O, int device) {
    if (!tensor || !y || !x || S <= 0 || I <= 0 || O <= 0 || !valid_fmt(fmt)) return 0;
    if (!g_initialized && !coli_runtime_init(NULL, 0)) return 0;
    if (!*tensor || (*tensor)->fmt != fmt || (*tensor)->I != I || (*tensor)->O != O || (*tensor)->device != device) {
        if (!coli_runtime_tensor_upload(tensor, weights, scales, fmt, I, O, device)) return 0;
    }
    const uint64_t input_hash = fnv1a(x, (size_t)S * (size_t)I * sizeof(float));
    if ((*tensor)->cache_y && (*tensor)->cache_fmt == fmt && (*tensor)->cache_S == S && (*tensor)->cache_I == I && (*tensor)->cache_O == O && (*tensor)->cache_hash == input_hash) {
        memcpy(y, (*tensor)->cache_y, (size_t)S * (size_t)O * sizeof(float));
        g_cache_hits += 1;
        return 1;
    }

    int lane_ids[4] = {0, 0, 0, 0};
    int lane_count = 1;
    lane_ids[0] = 0;
    if (g_backend_mask & 2) {
        lane_ids[lane_count++] = 1;
    }
    if (g_backend_mask & 4) {
        lane_ids[lane_count++] = 2;
    }
    if (g_backend_mask & 8) {
        lane_ids[lane_count++] = 3;
    }
    if (lane_count <= 0) lane_count = 1;

    const int base = O / lane_count;
    const int extra = O % lane_count;
    int *output_chunk_sizes = (int *)calloc((size_t)lane_count, sizeof(int));
    if (!output_chunk_sizes) return 0;
    for (int lane = 0; lane < lane_count; ++lane) {
        output_chunk_sizes[lane] = base + (lane < extra ? 1 : 0);
    }

    int dispatch_ok = 1;
    if (lane_count == 1) {
        dispatch_ok = dispatch_chunk(y, x, weights, scales, fmt, S, I, O, lane_ids[0], 0, O, device);
    } else {
        int local_start = 0;
        for (int lane = 0; lane < lane_count; ++lane) {
            if (!dispatch_chunk(y, x, weights, scales, fmt, S, I, O, lane_ids[lane], local_start, output_chunk_sizes[lane], device)) {
                dispatch_ok = 0;
            }
            local_start += output_chunk_sizes[lane];
        }
    }
    free(output_chunk_sizes);
    if (!dispatch_ok) return 0;

    free((*tensor)->cache_y);
    (*tensor)->cache_y = (float *)malloc((size_t)S * (size_t)O * sizeof(float));
    if ((*tensor)->cache_y) {
        memcpy((*tensor)->cache_y, y, (size_t)S * (size_t)O * sizeof(float));
        (*tensor)->cache_S = S;
        (*tensor)->cache_I = I;
        (*tensor)->cache_O = O;
        (*tensor)->cache_fmt = fmt;
        (*tensor)->cache_hash = input_hash;
    }
    return 1;
}

void coli_runtime_tensor_free(ColiCudaTensor *tensor) {
    if (!tensor) return;
    unregister_tensor(tensor);
    free(tensor->weights);
    free(tensor->scales);
    free(tensor->cache_y);
    free(tensor);
}

size_t coli_runtime_tensor_bytes(const ColiCudaTensor *tensor) {
    if (!tensor) return 0;
    return tensor->weight_bytes + tensor->scale_bytes;
}

int coli_runtime_tensor_device(const ColiCudaTensor *tensor) {
    return tensor ? tensor->device : -1;
}

#if !defined(COLI_ROCM) && !defined(COLI_ENABLE_NPU) && !defined(COLI_ENABLE_VULKAN)
int coli_cuda_init(const int *devices, int count) {
    return coli_runtime_init(devices, count);
}

void coli_cuda_shutdown(void) {
    coli_runtime_shutdown();
}

int coli_cuda_device_count(void) {
    return coli_runtime_device_count();
}

int coli_cuda_device_at(int index) {
    return coli_runtime_device_at(index);
}

int coli_cuda_mem_info(int device, size_t *free_bytes, size_t *total_bytes) {
    return coli_runtime_mem_info(device, free_bytes, total_bytes);
}

void coli_cuda_stats(int device, size_t *tensor_count, size_t *tensor_bytes) {
    coli_runtime_stats(device, tensor_count, tensor_bytes);
}

int coli_cuda_tensor_upload(ColiCudaTensor **tensor,
                            const void *weights, const float *scales,
                            int fmt, int I, int O, int device) {
    return coli_runtime_tensor_upload(tensor, weights, scales, fmt, I, O, device);
}

int coli_cuda_matmul(ColiCudaTensor **tensor,
                     float *y, const float *x,
                     const void *weights, const float *scales,
                     int fmt, int S, int I, int O, int device) {
    return coli_runtime_matmul(tensor, y, x, weights, scales, fmt, S, I, O, device);
}

void coli_cuda_tensor_free(ColiCudaTensor *tensor) {
    coli_runtime_tensor_free(tensor);
}

size_t coli_cuda_tensor_bytes(const ColiCudaTensor *tensor) {
    return coli_runtime_tensor_bytes(tensor);
}

int coli_cuda_tensor_device(const ColiCudaTensor *tensor) {
    return coli_runtime_tensor_device(tensor);
}
#endif

// ADDITIVE BACKENDS FIX
