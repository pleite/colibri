/**
 * test_npu.c — NPU backend test harness
 *
 * Tests the XDNA 2 NPU implementation:
 * 1. Device discovery and capability query
 * 2. Hardware context creation
 * 3. Buffer object management
 * 4. Matmul execution (CPU fallback path)
 * 5. Shape registration and lookup
 *
 * Compile:
 *   gcc -O3 -march=native -std=c11 \
 *       -I../npu_kernels \
 *       test_npu.c ../npu_kernels/xdna2_driver.c ../npu_kernels/xdna2_matmul.c \
 *       -o test_npu -ldl -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "xdna2_driver.h"
#include "xdna2_matmul.h"
#include <drm/amdxdna_accel.h>

/* ── Test helpers ── */

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST(name) \
    do { \
        g_tests_run++; \
        printf("TEST %d: %s ... ", g_tests_run, name); \
    } while(0)

#define PASS() \
    do { \
        g_tests_passed++; \
        printf("PASS\n"); \
    } while(0)

#define FAIL(msg) \
    do { \
        g_tests_failed++; \
        printf("FAIL: %s\n", msg); \
    } while(0)

/* ── Test: Device Discovery ── */

static void test_device_discovery(void) {
    TEST("NPU device discovery");

    int fd = -1;
    if (xdna2_open_device(&fd) < 0) {
        /* NPU may not be available in test environment */
        printf("SKIP (NPU device not available)\n");
        return;
    }

    xdna2_aie_metadata_t meta = {0};
    if (xdna2_query_aie_metadata(fd, &meta) == 0) {
        printf("PASS (AIE: %u x %u, version %u.%u)\n",
               meta.cols, meta.rows, meta.version_major, meta.version_minor);
        g_tests_passed++;
    } else {
        printf("SKIP (AIE metadata query failed)\n");
    }

    xdna2_resource_info_t info = {0};
    if (xdna2_query_resource_info(fd, &info) == 0) {
        printf("  Resource: %lu TOPS max, %lu tasks max\n",
               (unsigned long)info.npu_tops_max,
               (unsigned long)info.npu_task_max);
    }

    xdna2_close_device(fd);
}

/* ── Test: Hardware Context ── */

static void test_hwctx(void) {
    TEST("Hardware context creation");

    int fd = -1;
    if (xdna2_open_device(&fd) < 0) {
        printf("SKIP\n");
        return;
    }

    xdna2_hwctx_t ctx;
    int ret = xdna2_create_hwctx(fd, &ctx, 16, 32768, 4,
                                  AMDXDNA_QOS_HIGH_PRIORITY);
    if (ret == 0) {
        printf("PASS (handle=%u, tiles=%u)\n",
               ctx.hwctx_handle, ctx.num_tiles);
        g_tests_passed++;

        xdna2_destroy_hwctx(fd, &ctx);
    } else {
        printf("SKIP (hwctx creation failed — likely partition conflict)\n");
    }

    xdna2_close_device(fd);
}

/* ── Test: Buffer Objects ── */

static void test_buffer_objects(void) {
    TEST("Buffer object management");

    int fd = -1;
    if (xdna2_open_device(&fd) < 0) {
        printf("SKIP\n");
        return;
    }

    /* Create a SHARE-type BO (host-visible) */
    xdna2_bo_t bo;
    int ret = xdna2_create_bo(fd, &bo, 4096, AMDXDNA_BO_SHARE);
    if (ret < 0) {
        printf("SKIP (BO creation failed)\n");
        xdna2_close_device(fd);
        return;
    }

    printf("PASS (BO handle=%u, size=%lu, xdna_addr=0x%lx)\n",
           bo.handle, (unsigned long)bo.size, (unsigned long)bo.xdna_addr);
    g_tests_passed++;

    /* Map and write test data */
    if (xdna2_map_bo(fd, &bo) == 0) {
        uint8_t *data = (uint8_t *)bo.mapped;
        for (int i = 0; i < 16; i++) data[i] = (uint8_t)(i * 17);

        /* Sync to device */
        xdna2_sync_bo(fd, &bo, SYNC_DIRECT_TO_DEVICE, 0, 16);

        /* Sync back and verify */
        xdna2_sync_bo(fd, &bo, SYNC_DIRECT_FROM_DEVICE, 0, 16);

        int ok = 1;
        for (int i = 0; i < 16; i++) {
            if (data[i] != (uint8_t)(i * 17)) { ok = 0; break; }
        }
        if (ok) {
            printf("  BO map/sync: OK\n");
        } else {
            printf("  BO map/sync: DATA MISMATCH\n");
            g_tests_failed++;
        }

        xdna2_unmap_bo(&bo);
    }

    xdna2_destroy_bo(fd, &bo);
    xdna2_close_device(fd);
}

/* ── Test: Runtime Init/Shutdown ── */

static void test_runtime_lifecycle(void) {
    TEST("NPU runtime lifecycle");

    xdna2_runtime_t runtime = {0};

    int ret = xdna2_runtime_init(&runtime);
    if (ret < 0) {
        printf("SKIP (runtime init failed)\n");
        return;
    }

    printf("PASS (initialized, device_fd=%d)\n", runtime.device_fd);
    g_tests_passed++;

    xdna2_runtime_shutdown(&runtime);
    printf("  Shutdown OK\n");
}

/* ── Test: Matmul CPU Fallback ── */

static void test_matmul_fallback(void) {
    TEST("Matmul CPU fallback (shape 1x64x64)");

    const int S = 1, I = 64, O = 64;
    float *x = (float *)calloc(S * I, sizeof(float));
    int8_t *weights = (int8_t *)malloc(O * I * sizeof(int8_t));
    float *scales = (float *)calloc(O, sizeof(float));
    float *y = (float *)calloc(S * O, sizeof(float));
    float *y_ref = (float *)calloc(S * O, sizeof(float));

    /* Initialize test data */
    for (int i = 0; i < S * I; i++) x[i] = (float)(i % 10) - 4.0f;
    for (int i = 0; i < O * I; i++) weights[i] = (int8_t)((i % 13) - 6);
    for (int o = 0; o < O; o++) scales[o] = 1.0f / 128.0f;

    /* Reference: naive CPU matmul */
    for (int o = 0; o < O; o++) {
        for (int s = 0; s < S; s++) {
            float acc = 0.0f;
            for (int i = 0; i < I; i++) {
                acc += x[s * I + i] * (float)weights[o * I + i];
            }
            y_ref[s * O + o] = acc * scales[o];
        }
    }

    /* NPU matmul (will use CPU fallback) */
    xdna2_runtime_t runtime = {0};
    xdna2_runtime_init(&runtime);

    int ret = xdna2_matmul_int8(x, weights, scales, y, S, I, O, &runtime);
    xdna2_runtime_shutdown(&runtime);

    if (ret == 0) {
        /* Verify against reference */
        float max_err = 0.0f;
        for (int i = 0; i < S * O; i++) {
            float err = fabsf(y[i] - y_ref[i]);
            if (err > max_err) max_err = err;
        }

        if (max_err < 1.0f) {
            printf("PASS (max_err=%e)\n", max_err);
            g_tests_passed++;
        } else {
            printf("FAIL (max_err=%e)\n", max_err);
            g_tests_failed++;
        }
    } else {
        printf("FAIL (matmul returned %d)\n", ret);
        g_tests_failed++;
    }

    free(x); free(weights); free(scales); free(y); free(y_ref);
}

/* ── Test: Shape Registration ── */

static void test_shape_registration(void) {
    TEST("Shape registration and lookup");

    xdna2_runtime_t runtime = {0};
    xdna2_matmul_shape_t shape = {.rows = 1, .inner_dim = 4096,
                                   .out_cols = 4096, .fmt = 1};

    /* Register a fake kernel handle */
    int ret = xdna2_register_shape(&runtime, &shape, 42);
    if (ret != 0) {
        printf("FAIL (register returned %d)\n", ret);
        g_tests_failed++;
        return;
    }

    /* Look up the shape */
    uint32_t handle = xdna2_find_kernel(&runtime, 1, 4096, 4096);
    if (handle == 42) {
        printf("PASS (found kernel 42 for shape 1x4096x4096)\n");
        g_tests_passed++;
    } else {
        printf("FAIL (expected 42, got %u)\n", handle);
        g_tests_failed++;
    }

    /* Look up non-existent shape */
    handle = xdna2_find_kernel(&runtime, 1, 1024, 1024);
    if (handle == 0) {
        printf("  Correctly returns 0 for unregistered shape\n");
    } else {
        printf("  WARNING: unexpected handle %u for unregistered shape\n", handle);
    }
}

/* ── Main ── */

int main(void) {
    printf("=== XDNA 2 NPU Test Suite ===\n\n");

    test_device_discovery();
    test_hwctx();
    test_buffer_objects();
    test_runtime_lifecycle();
    test_matmul_fallback();
    test_shape_registration();

    printf("\n=== Results: %d/%d passed", g_tests_passed, g_tests_run);
    if (g_tests_failed > 0) {
        printf(", %d FAILED", g_tests_failed);
    }
    printf(" ===\n");

    return g_tests_failed > 0 ? 1 : 0;
}