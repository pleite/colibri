#include <stddef.h>
#include <stdint.h>

int coli_vulkan_native_init(void) {
    return 1;
}

void coli_vulkan_native_shutdown(void) {
}

int coli_vulkan_native_matmul(const void *weights, const float *scales, int fmt, int S, int I, int O, float *y, const float *x, int device) {
    (void)weights;
    (void)scales;
    (void)fmt;
    (void)S;
    (void)I;
    (void)O;
    (void)device;
    if (!y || !x) return 0;
    for (int i = 0; i < O * S; ++i) {
        y[i] = 42.0f + (float)i;
    }
    return 1;
}

int coli_npu_native_init(void) {
    return 1;
}

void coli_npu_native_shutdown(void) {
}

int coli_npu_native_matmul(const void *weights, const float *scales, int fmt, int S, int I, int O, float *y, const float *x, int device) {
    (void)weights;
    (void)scales;
    (void)fmt;
    (void)S;
    (void)I;
    (void)O;
    (void)device;
    if (!y || !x) return 0;
    for (int i = 0; i < O * S; ++i) {
        y[i] = 77.0f + (float)i;
    }
    return 1;
}
