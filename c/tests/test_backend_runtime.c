#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../backend_runtime.h"

static int close_enough(float a, float b) {
    return fabsf(a - b) < 1e-5f;
}

int main(void) {
    int devices[] = {0};
    if (!coli_runtime_init(devices, 1)) {
        return 77;
    }

    float x[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float weights[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
    };
    float scales[] = {1.0f, 1.25f};
    float out[2] = {0.0f, 0.0f};
    float out_again[2] = {0.0f, 0.0f};
    ColiCudaTensor *tensor = NULL;

    if (!coli_runtime_tensor_upload(&tensor, weights, scales, 0, 4, 2, 0)) {
        return 1;
    }
    if (!coli_runtime_matmul(&tensor, out, x, weights, scales, 0, 1, 4, 2, 0)) {
        return 1;
    }
    if (!coli_runtime_matmul(&tensor, out_again, x, weights, scales, 0, 1, 4, 2, 0)) {
        return 1;
    }

    if (!close_enough(out[0], 30.0f) || !close_enough(out[1], 70.0f)) {
        fprintf(stderr, "unexpected output: %.5f %.5f\n", out[0], out[1]);
        return 1;
    }
    if (!close_enough(out_again[0], 30.0f) || !close_enough(out_again[1], 70.0f)) {
        fprintf(stderr, "unexpected cached output: %.5f %.5f\n", out_again[0], out_again[1]);
        return 1;
    }
    if (coli_runtime_cache_hits() == 0) {
        fprintf(stderr, "expected cache hit after repeated batched execution\n");
        return 1;
    }

    coli_runtime_tensor_free(tensor);
    coli_runtime_shutdown();
    return 0;
}
