/**
 * xdna2_backend.c — Strix Halo XDNA 2 NPU backend for the VNNI int8 matmul
 * building blocks.
 *
 * No CPU fallback. See npu/xdna2_backend.h.
 */

#include "xdna2_backend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../../c/npu_kernels/xdna2_matmul.h"

static xdna2_runtime_t g_runtime;
static int g_init_attempted = 0;
static int g_ready = 0;
static const char *g_failure_reason = "not initialised";

static const char *kernel_dir(void) {
    const char *dir = getenv("COLI_NPU_KERNEL_DIR");
    return (dir && dir[0]) ? dir : "npu/kernels";
}

/*
 * Load every kernel artifact listed in COLI_NPU_KERNELS (a colon-separated list
 * of paths). When the variable is unset nothing is preloaded and kernels are
 * resolved lazily by shape.
 */
static void preload_kernels(void) {
    const char *list = getenv("COLI_NPU_KERNELS");
    if (!list || !list[0]) return;

    char *copy = strdup(list);
    if (!copy) return;
    for (char *tok = strtok(copy, ":"); tok; tok = strtok(NULL, ":")) {
        if (tok[0]) {
            xdna2_load_kernel(&g_runtime, tok);
        }
    }
    free(copy);
}

static int ensure_runtime(void) {
    if (g_init_attempted) {
        return g_ready;
    }
    g_init_attempted = 1;

    int ret = xdna2_runtime_init(&g_runtime);
    if (ret < 0) {
        switch (ret) {
            case -ENODEV:
                g_failure_reason =
                    "cannot open /dev/accel/accel0 (is the amdxdna module loaded "
                    "and the device passed into the container?)";
                break;
            case -ENOSYS:
                g_failure_reason =
                    "XDNA2_DRIVER=xrt was requested but the XRT/XDNA shim stack "
                    "is unusable on this host";
                break;
            case -EINVAL:
                g_failure_reason = "XDNA2_DRIVER is not auto, drm or xrt";
                break;
            default:
                g_failure_reason = "failed to create an XDNA 2 hardware context";
                break;
        }
        return 0;
    }

    preload_kernels();
    g_ready = 1;
    g_failure_reason = "none";
    return 1;
}

/* Resolve a kernel for this exact shape, loading it from disk on first use. */
static int ensure_kernel_for_shape(int rows, int inner_dim, int out_cols) {
    if (xdna2_find_kernel(&g_runtime, rows, inner_dim, out_cols, XDNA2_FMT_INT8)) {
        return 1;
    }

    char path[512];
    int n = snprintf(path, sizeof(path), "%s/matmul_int8_%dx%dx%d.npukernel",
                     kernel_dir(), rows, inner_dim, out_cols);
    if (n <= 0 || (size_t)n >= sizeof(path)) {
        return 0;
    }
    if (xdna2_load_kernel(&g_runtime, path) < 0) {
        return 0;
    }
    return xdna2_find_kernel(&g_runtime, rows, inner_dim, out_cols, XDNA2_FMT_INT8) != NULL;
}

int strix_xdna2_is_supported(void) {
    return ensure_runtime();
}

int strix_xdna2_matmul(const int8_t *input,
                       int rows,
                       int inner_dim,
                       const int8_t *weights,
                       int out_cols,
                       float *output,
                       const float *scales) {
    if (!input || !weights || !output || rows <= 0 || inner_dim <= 0 || out_cols <= 0) {
        return 0;
    }
    if (!ensure_runtime()) {
        return 0;
    }
    if (!ensure_kernel_for_shape(rows, inner_dim, out_cols)) {
        fprintf(stderr,
                "xdna2 backend: no kernel for shape (%d, %d, %d); "
                "AIE-2 is fixed-shape and this backend has no CPU fallback\n",
                rows, inner_dim, out_cols);
        return 0;
    }
    return xdna2_matmul_int8(&g_runtime, input, weights, scales, output,
                             rows, inner_dim, out_cols) == 0;
}

const char *strix_xdna2_backend_name(void) {
    return ensure_runtime() ? "xdna2-npu" : "xdna2-unavailable";
}

const char *strix_xdna2_failure_reason(void) {
    return g_failure_reason;
}

void strix_xdna2_shutdown(void) {
    if (g_init_attempted && g_ready) {
        xdna2_runtime_shutdown(&g_runtime);
    }
    g_init_attempted = 0;
    g_ready = 0;
    g_failure_reason = "not initialised";
}
