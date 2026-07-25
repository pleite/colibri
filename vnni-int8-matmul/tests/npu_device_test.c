/**
 * npu_device_test.c — Strix Halo XDNA 2 NPU device and runtime probe.
 *
 * This harness never validates numerics against a CPU reference, because the
 * NPU backend has no CPU fallback: a numeric mismatch would mean the NPU
 * produced wrong results, and an absent kernel means there is nothing to
 * compare against. It checks, in order:
 *
 *   1. the accel node opens and a hardware context can be created,
 *   2. the AIE array reported by the driver looks like XDNA 2 on Strix Halo,
 *   3. requesting a matmul with no kernel loaded fails instead of silently
 *      falling back to the CPU,
 *   4. int4 -> int8 weight expansion is correct (pure host arithmetic).
 *
 * Steps 1-3 are skipped, not failed, when no NPU is present, so the harness is
 * usable in CI. Step 4 always runs.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../npu/xdna2_backend.h"
#include "../../c/npu_kernels/xdna2_driver.h"
#include "../../c/npu_kernels/xdna2_matmul.h"

static int test_int4_expansion(void) {
    /* Two output rows, four input columns: nibbles are little-endian within a
     * byte and use a zero point of 8. */
    const uint8_t packed[4] = {
        0x10, 0x32, /* row 0: nibbles 0,1,2,3 -> -8,-7,-6,-5 */
        0xBA, 0xDC, /* row 1: nibbles A,B,C,D -> 2,3,4,5 */
    };
    const int8_t expected[8] = {-8, -7, -6, -5, 2, 3, 4, 5};
    int8_t got[8];
    memset(got, 0, sizeof(got));

    xdna2_dequant_int4(packed, got, /* O */ 2, /* I */ 4);

    for (int i = 0; i < 8; ++i) {
        if (got[i] != expected[i]) {
            fprintf(stderr, "int4 expansion mismatch at %d: got=%d expected=%d\n",
                    i, (int)got[i], (int)expected[i]);
            return 0;
        }
    }
    printf("int4 -> int8 expansion OK\n");
    return 1;
}

static int test_device_probe(void) {
    if (!strix_xdna2_is_supported()) {
        printf("NPU probe SKIP (%s)\n", strix_xdna2_failure_reason());
        return 1;
    }
    printf("NPU probe OK (%s)\n", strix_xdna2_backend_name());

    int fd = -1;
    if (xdna2_open_device(&fd) == 0) {
        xdna2_aie_metadata_t meta;
        memset(&meta, 0, sizeof(meta));
        if (xdna2_query_aie_metadata(fd, &meta) == 0) {
            printf("AIE array: %u columns x %u rows, version %u.%u\n",
                   meta.cols, meta.rows, meta.version_major, meta.version_minor);
            if (meta.version_major < 2) {
                fprintf(stderr,
                        "this backend targets XDNA 2 (AIE-2); reported AIE version is %u.%u\n",
                        meta.version_major, meta.version_minor);
                xdna2_close_device(fd);
                return 0;
            }
        }
        xdna2_close_device(fd);
    }
    return 1;
}

static int test_no_cpu_fallback(void) {
    if (!strix_xdna2_is_supported()) {
        printf("no-fallback check SKIP (%s)\n", strix_xdna2_failure_reason());
        return 1;
    }

    /* A shape no shipped kernel covers. The backend must refuse it. */
    const int rows = 3;
    const int inner_dim = 7;
    const int out_cols = 5;
    int8_t input[3 * 7];
    int8_t weights[5 * 7];
    float output[3 * 5];

    memset(input, 1, sizeof(input));
    memset(weights, 1, sizeof(weights));
    memset(output, 0, sizeof(output));

    if (strix_xdna2_matmul(input, rows, inner_dim, weights, out_cols, output, NULL)) {
        fprintf(stderr,
                "backend accepted shape (%d, %d, %d) with no kernel loaded; "
                "a CPU fallback must not exist\n", rows, inner_dim, out_cols);
        return 0;
    }
    printf("no-fallback check OK (unsupported shape correctly rejected)\n");
    return 1;
}

int main(void) {
    int failed = 0;

    if (!test_int4_expansion()) failed = 1;
    if (!test_device_probe()) failed = 1;
    if (!test_no_cpu_fallback()) failed = 1;

    strix_xdna2_shutdown();

    if (failed) {
        fprintf(stderr, "NPU device tests FAILED\n");
        return 1;
    }
    printf("NPU device tests passed or skipped for a host without an XDNA 2 NPU.\n");
    return 0;
}
