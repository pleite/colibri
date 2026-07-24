#include "vulkan_backend.h"

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../cpu/vnni_cpu_backend.h"

static int g_vulkan_available = 0;

static void probe_vulkan(void) {
    void *handle = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        handle = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    }
    if (handle) {
        dlclose(handle);
        g_vulkan_available = 1;
        return;
    }
    g_vulkan_available = 0;
}

int strix_vulkan_matmul(const int8_t *input,
                        int rows,
                        int inner_dim,
                        const int8_t *weights,
                        int out_cols,
                        float *output,
                        const float *scales) {
    static int initialized = 0;
    if (!initialized) {
        probe_vulkan();
        initialized = 1;
    }

    if (!g_vulkan_available) {
        fprintf(stderr, "vulkan backend: using CPU fallback because no Vulkan loader is available\n");
    }

    return strix_cpu_matmul(input, rows, inner_dim, weights, out_cols, output, scales);
}

const char *strix_vulkan_backend_name(void) {
    return g_vulkan_available ? "vulkan-dispatch" : "vulkan-cpu-fallback";
}
