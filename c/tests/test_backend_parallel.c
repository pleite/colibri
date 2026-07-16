/* Multi-lane parallel scheduler test.
 *
 * Compiled with -DCOLI_ENABLE_NPU and linked against backend_npu.c so that the
 * runtime sees TWO active lanes (cpu + npu). The NPU shim computes on the CPU,
 * so the numeric result is independent of how the output rows are partitioned
 * across lanes. That lets us validate:
 *   - parallel (pthreads) dispatch produces the same result as sequential,
 *   - role-aware partitioning (AUTO / ATTENTION / SMALL) is numerically correct,
 *   - the reporting API reflects the active backends.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../backend_runtime.h"

static int close_enough(float a, float b) { return fabsf(a - b) < 1e-4f; }

/* Reference dense matmul: y[o] = sum_i x[i] * w[o*I+i]. */
static void reference(const float *x, const float *w, int I, int O, float *y) {
    for (int o = 0; o < O; ++o) {
        float acc = 0.0f;
        for (int i = 0; i < I; ++i) acc += x[i] * w[(size_t)o * I + i];
        y[o] = acc;
    }
}

static int run_role(coli_op_role role, const float *x, const float *w,
                    int I, int O, const float *expected) {
    float out[64] = {0};
    ColiCudaTensor *t = NULL;
    if (!coli_runtime_matmul_ex(&t, out, x, w, NULL, 0, 1, I, O, 0, role)) {
        fprintf(stderr, "matmul_ex failed for role %d\n", (int)role);
        if (t) coli_runtime_tensor_free(t);
        return 1;
    }
    for (int o = 0; o < O; ++o) {
        if (!close_enough(out[o], expected[o])) {
            fprintf(stderr, "role %d row %d mismatch: got %.5f expected %.5f\n",
                    (int)role, o, out[o], expected[o]);
            coli_runtime_tensor_free(t);
            return 1;
        }
    }
    coli_runtime_tensor_free(t);
    return 0;
}

int main(void) {
    if (setenv("COLI_RUNTIME_ENGINES", "cpu,npu", 1) != 0) return 77;
    if (setenv("COLI_RUNTIME_DISABLE_ENGINES", "vulkan,rocm,cuda", 1) != 0) return 77;

    int devices[] = {0};
    if (!coli_runtime_init(devices, 1)) return 77;

    /* Two lanes expected: cpu + npu. */
    char names[64] = {0};
    int active = coli_runtime_active_backends(names, sizeof(names));
    if (active < 2 || !strstr(names, "cpu") || !strstr(names, "npu")) {
        fprintf(stderr, "expected cpu+npu lanes, got %d (%s)\n", active, names);
        return 1;
    }

    const int I = 4, O = 8;
    float x[4] = {1.0f, -2.0f, 0.5f, 3.0f};
    float w[32];
    for (int o = 0; o < O; ++o)
        for (int i = 0; i < I; ++i) w[o * I + i] = (float)(o + 1) * 0.1f + (float)i;
    float expected[8];
    reference(x, w, I, O, expected);

    /* Parallel dispatch across both lanes, several roles. */
    coli_op_role roles[] = {COLI_OP_ROLE_AUTO, COLI_OP_ROLE_ATTENTION,
                            COLI_OP_ROLE_SHARED_EXPERT, COLI_OP_ROLE_ROUTED_EXPERT,
                            COLI_OP_ROLE_DENSE, COLI_OP_ROLE_SMALL};
    for (size_t r = 0; r < sizeof(roles) / sizeof(roles[0]); ++r) {
        if (run_role(roles[r], x, w, I, O, expected)) return 1;
    }

    /* Disable parallelism and confirm identical results (sequential path). */
    coli_runtime_shutdown();
    if (setenv("COLI_RUNTIME_PARALLEL", "0", 1) != 0) return 77;
    if (!coli_runtime_init(devices, 1)) return 77;
    if (coli_runtime_parallel_enabled() != 0) {
        fprintf(stderr, "parallel should be disabled\n");
        return 1;
    }
    for (size_t r = 0; r < sizeof(roles) / sizeof(roles[0]); ++r) {
        if (run_role(roles[r], x, w, I, O, expected)) return 1;
    }

    coli_runtime_shutdown();
    puts("backend parallel test: ok");
    return 0;
}
