#include "../backend_vulkan.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int close_enough(const float *got, const float *want, int n) {
    for (int i = 0; i < n; ++i) {
        if (fabsf(got[i] - want[i]) > 1e-4f) return 0;
    }
    return 1;
}

int main(void) {
    int devices[COLI_CUDA_MAX_DEVICES] = {0};
    if (!coli_cuda_init(devices, 1)) return 77;
    if (coli_cuda_device_count() != 1) return 1;
    if (coli_cuda_device_at(0) != 0) return 1;

    const float x[4] = {1.0f, -2.0f, 3.0f, 0.5f};
    const float w_f32[4] = {0.25f, 1.5f, -0.5f, 2.0f};
    float got[2] = {0.0f, 0.0f};
    float want[2] = {0.0f, 0.0f};
    ColiCudaTensor *tensor = NULL;

    if (!coli_cuda_tensor_upload(&tensor, w_f32, NULL, 0, 4, 1, 0)) return 1;
    if (!coli_cuda_matmul(&tensor, got, x, w_f32, NULL, 0, 1, 4, 1, 0)) return 1;
    want[0] = x[0] * w_f32[0] + x[1] * w_f32[1] + x[2] * w_f32[2] + x[3] * w_f32[3];
    if (!close_enough(got, want, 1)) return 1;

    const int8_t q8[4] = {1, -2, 3, -4};
    const float scales[1] = {0.125f};
    float got8[1] = {0.0f};
    if (!coli_cuda_matmul(&tensor, got8, x, q8, scales, 1, 1, 4, 1, 0)) return 1;
    want[0] = (x[0] * 1 + x[1] * -2 + x[2] * 3 + x[3] * -4) * 0.125f;
    if (!close_enough(got8, want, 1)) return 1;

    coli_cuda_tensor_free(tensor);
    coli_cuda_shutdown();
    return 0;
}
