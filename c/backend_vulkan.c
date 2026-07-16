#include "backend_vulkan.h"

#include <stdbool.h>
#include <dlfcn.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "parallelism.h"

#ifdef _OPENMP
#include <omp.h>
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
};

/*
 * Return host memory availability in bytes for the backend capability report.
 * Returns 1 on success and 0 on failure. When provided, free_bytes and
 * total_bytes hold the available and total host memory, respectively.
 */
static int host_memory_info(size_t *free_bytes, size_t *total_bytes) {
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    if (pages <= 0 || page_size <= 0) return 0;
    if ((size_t)pages > SIZE_MAX / (size_t)page_size) return 0;
    const size_t total = (size_t)pages * (size_t)page_size;
    if (total_bytes) *total_bytes = total;
#ifdef _SC_AVPHYS_PAGES
    long avail_pages = sysconf(_SC_AVPHYS_PAGES);
    if (avail_pages <= 0) avail_pages = pages;
#else
    long avail_pages = pages;
#endif
    if ((size_t)avail_pages > SIZE_MAX / (size_t)page_size) return 0;
    if (free_bytes) *free_bytes = (size_t)avail_pages * (size_t)page_size;
    return 1;
}

static int g_initialized = 0;
static int g_devices[COLI_CUDA_MAX_DEVICES];
static int g_device_count = 0;
static size_t g_tensor_count = 0;
static size_t g_tensor_bytes = 0;
static ColiCudaTensor **g_tensors = NULL;
static size_t g_tensor_capacity = 0;
static _Atomic bool g_parallelism_configured = false;
static int g_vulkan_loader_available = 0;

static int env_truthy(const char *name) {
    const char *value = getenv(name);
    if (!value || !*value) return 0;
    return !(strcasecmp(value, "0") == 0 || strcasecmp(value, "false") == 0 || strcasecmp(value, "off") == 0);
}

static void configure_parallelism(void) {
    bool expected = false;
    if (!atomic_compare_exchange_strong(&g_parallelism_configured, &expected, true)) return;
#ifdef _OPENMP
    int threads = coli_detect_thread_count();
    omp_set_dynamic(0);
    omp_set_num_threads(threads);
#endif
}

static void probe_loader(void) {
    if (env_truthy("COLI_VULKAN_DISABLE")) {
        g_vulkan_loader_available = 0;
        return;
    }
    void *handle = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        handle = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    }
    if (handle) {
        dlclose(handle);
        g_vulkan_loader_available = 1;
    } else {
        g_vulkan_loader_available = 0;
    }
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
            for (size_t j = i + 1; j < g_tensor_count; ++j) g_tensors[j - 1] = g_tensors[j];
            g_tensor_count -= 1;
            g_tensor_bytes -= tensor->weight_bytes + tensor->scale_bytes;
            return;
        }
    }
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

static int decode_i4(uint8_t packed, int index) {
    int nibble = (index & 1) ? (packed >> 4) : (packed & 0x0f);
    return (int)(nibble & 0x0f) - 8;
}

static int decode_i2(uint8_t packed, int index) {
    int shift = (index & 3) * 2;
    return ((packed >> shift) & 0x03) - 2;
}

static void matmul_host(float *y, const float *x, const ColiCudaTensor *tensor, int S, int I, int O) {
    const float *weights_f32 = (const float *)tensor->weights;
    const int8_t *weights_i8 = (const int8_t *)tensor->weights;
    const uint8_t *weights_packed = (const uint8_t *)tensor->weights;
    const float *scales = tensor->scales;

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
    for (int o = 0; o < O; ++o) {
        const float scale = (scales && tensor->fmt != 0) ? scales[o] : 1.0f;
        for (int s = 0; s < S; ++s) {
            const float *row = x + (size_t)s * (size_t)I;
            float acc = 0.0f;
            for (int i = 0; i < I; ++i) {
                if (tensor->fmt == 0) {
                    acc += row[i] * weights_f32[(size_t)o * (size_t)I + i];
                } else if (tensor->fmt == 1) {
                    acc += row[i] * (float)weights_i8[(size_t)o * (size_t)I + i];
                } else if (tensor->fmt == 2) {
                    const int idx = i;
                    const int value = decode_i4(weights_packed[(size_t)o * (size_t)((I + 1) / 2) + (idx >> 1)], idx);
                    acc += row[i] * (float)value;
                } else {
                    const int idx = i;
                    const int value = decode_i2(weights_packed[(size_t)o * (size_t)((I + 3) / 4) + (idx >> 2)], idx);
                    acc += row[i] * (float)value;
                }
            }
            y[(size_t)s * (size_t)O + o] = acc * scale;
        }
    }
}

int coli_cuda_init(const int *devices, int count) {
    if (!devices || count <= 0) {
        g_devices[0] = 0;
        g_device_count = 1;
    } else {
        g_device_count = count > COLI_CUDA_MAX_DEVICES ? COLI_CUDA_MAX_DEVICES : count;
        for (int i = 0; i < g_device_count; ++i) g_devices[i] = devices[i];
    }
    configure_parallelism();
    probe_loader();
    g_initialized = 1;
    return 1;
}

void coli_cuda_shutdown(void) {
    while (g_tensor_count > 0) {
        coli_cuda_tensor_free(g_tensors[0]);
    }
    free(g_tensors);
    g_tensors = NULL;
    g_tensor_capacity = 0;
    g_initialized = 0;
    g_device_count = 0;
    g_tensor_count = 0;
    g_tensor_bytes = 0;
    g_vulkan_loader_available = 0;
}

int coli_cuda_device_count(void) {
    return g_initialized ? g_device_count : 0;
}

int coli_cuda_device_at(int index) {
    if (!g_initialized || index < 0 || index >= g_device_count) return -1;
    return g_devices[index];
}

int coli_cuda_mem_info(int device, size_t *free_bytes, size_t *total_bytes) {
    (void)device;
    return host_memory_info(free_bytes, total_bytes);
}

void coli_cuda_stats(int device, size_t *tensor_count, size_t *tensor_bytes) {
    (void)device;
    if (tensor_count) *tensor_count = g_tensor_count;
    if (tensor_bytes) *tensor_bytes = g_tensor_bytes;
}

int coli_cuda_tensor_upload(ColiCudaTensor **tensor,
                             const void *weights, const float *scales,
                             int fmt, int I, int O, int device) {
    if (!tensor || !valid_fmt(fmt) || I <= 0 || O <= 0 || device < 0) return 0;
    const int has_valid_payload = weights != NULL && (fmt == 0 || scales != NULL);
    if (!has_valid_payload) return 0;
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
            coli_cuda_tensor_free(*tensor);
            *tensor = NULL;
            return 0;
        }
        memcpy((*tensor)->weights, weights, (*tensor)->weight_bytes);
    }

    if ((*tensor)->scale_bytes > 0) {
        (*tensor)->scales = (float *)malloc((*tensor)->scale_bytes);
        if (!(*tensor)->scales) {
            coli_cuda_tensor_free(*tensor);
            *tensor = NULL;
            return 0;
        }
        memcpy((*tensor)->scales, scales, (*tensor)->scale_bytes);
    }

    register_tensor(*tensor);
    return 1;
}

int coli_cuda_matmul(ColiCudaTensor **tensor,
                     float *y, const float *x,
                     const void *weights, const float *scales,
                     int fmt, int S, int I, int O, int device) {
    if (!tensor || !y || !x || S <= 0 || I <= 0 || O <= 0 || !valid_fmt(fmt)) return 0;
    if (!*tensor || (*tensor)->fmt != fmt || (*tensor)->I != I || (*tensor)->O != O || (*tensor)->device != device) {
        if (!coli_cuda_tensor_upload(tensor, weights, scales, fmt, I, O, device)) return 0;
    }
    matmul_host(y, x, *tensor, S, I, O);
    return 1;
}

void coli_cuda_tensor_free(ColiCudaTensor *tensor) {
    if (!tensor) return;
    unregister_tensor(tensor);
    free(tensor->weights);
    free(tensor->scales);
    free(tensor);
}

size_t coli_cuda_tensor_bytes(const ColiCudaTensor *tensor) {
    if (!tensor) return 0;
    return tensor->weight_bytes + tensor->scale_bytes;
}

int coli_cuda_tensor_device(const ColiCudaTensor *tensor) {
    return tensor ? tensor->device : -1;
}
