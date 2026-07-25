/**
 * xdna2_matmul.c — NPU matmul implementation for XDNA 2
 *
 * Fixed-shape int8 matrix multiplication via XDNA 2 AIE-2.
 * Falls back to CPU for unsupported shapes or when aiecompiler
 * is not available.
 *
 * Strix Halo: XDNA 2, ~30 TOPS INT8, fixed-shape AIE-2 tile ISA
 */

#include "xdna2_matmul.h"
#include "xdna2_driver.h"
#include <drm/amdxdna_accel.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* ── CPU fallback (local implementation for standalone testing) ──
 * In production, this would link against the AVX-512 VNNI kernel
 * from cpu_kernels/ or vnni_kernels/. For standalone testing,
 * we provide a portable scalar fallback.
 */

static void strix_cpu_matmul(const int8_t *input, int rows, int inner_dim,
                              const int8_t *weights, int out_cols,
                              float *output, const float *scales) {
    for (int o = 0; o < out_cols; o++) {
        const float scale = scales ? scales[o] : 1.0f;
        for (int s = 0; s < rows; s++) {
            float acc = 0.0f;
            for (int i = 0; i < inner_dim; i++) {
                acc += (float)input[(size_t)s * (size_t)inner_dim + i] *
                       (float)weights[(size_t)o * (size_t)inner_dim + i];
            }
            output[(size_t)s * (size_t)out_cols + o] = acc * scale;
        }
    }
}

/* ── AIE kernel source template ── */

/*
 * The AIE kernel for int8 matmul would be written in AIE C (part of
 * the AIE compiler toolchain). It operates on the AIE-2 tile ISA which
 * has:
 *   - 16-bit MAC units (4 MACs per tile cycle)
 *   - 16KB local memory per tile
 *   - Lock/Event-based synchronization
 *   - DMA channels for data movement
 *
 * Example AIE kernel structure (not compiled here — requires aiecompiler):
 *
 *   #include "aie_api/aie_api.hpp"
 *   #include "aie_api/aie_matmul.hpp"
 *
 *   constexpr int32_t ROWS = 1;
 *   constexpr int32_t INNER = 4096;
 *   constexpr int32_t OUT = 4096;
 *
 *   void matmul_int8_kernel(
 *       const int8_t* input,
 *       const int8_t* weights,
 *       const float* scales,
 *       float* output)
 *   {
 *       // AIE-2 tile ISA: fixed-shape, no dynamic allocation
 *       // Each tile processes a sub-block of the matrix
 *       // Lock-based synchronization between tiles
 *       // DMA for loading input/weights from host memory
 *
 *       for (int o = 0; o < OUT; o++) {
 *           for (int s = 0; s < ROWS; s++) {
 *               float acc = 0.0f;
 *               for (int i = 0; i < INNER; i++) {
 *                   acc += (float)input[s * INNER + i] *
 *                          (float)weights[o * INNER + i];
 *               }
 *               output[s * OUT + o] = acc * scales[o];
 *           }
 *       }
 *   }
 *
 * This would be compiled with:
 *   aiecompiler-mlir --target=aie --aie-version=2 \
 *       -DROWS=1 -DINNER=4096 -DOUT=4096 \
 *       matmul_int8.aie.cpp -o matmul_int8.aie
 *
 * Then packaged:
 *   aiecompiler-mlir --package matmul_int8.aie -o matmul_int8.xclbin
 */

/* ── Kernel Compilation (stub — requires aiecompiler) ── */

uint32_t xdna2_compile_kernel(const xdna2_matmul_shape_t *shape,
                               const char *output_path) {
    (void)output_path;

    if (!shape) return 0;

    fprintf(stderr, "xdna2: kernel compilation requires aiecompiler\n");
    fprintf(stderr, "xdna2: shape: rows=%d, inner=%d, out=%d, fmt=%d\n",
            shape->rows, shape->inner_dim, shape->out_cols, shape->fmt);
    fprintf(stderr, "xdna2: aiecompiler is not available — using CPU fallback\n");

    /*
     * When aiecompiler is installed, this function would:
     * 1. Generate AIE C source with compile-time constants
     * 2. Call aiecompiler-mlir to compile to .aie package
     * 3. Package to .xclbin
     * 4. Load into NPU via DRM ioctl
     * 5. Return kernel handle
     *
     * For now, return 0 (no kernel compiled).
     */
    return 0;
}

uint32_t xdna2_load_kernel(const char *kernel_path,
                            const xdna2_matmul_shape_t *shape) {
    if (!kernel_path || !shape) return 0;

    /*
     * Load a pre-compiled .xclbin or .aie package.
     * This would use the DRM ioctl AMDXDNA_EXEC_CMD to load
     * the kernel binary into the NPU firmware.
     *
     * For now, return 0 (no kernel loaded).
     */
    fprintf(stderr, "xdna2: kernel loading from '%s' not yet implemented\n",
            kernel_path);
    return 0;
}

void xdna2_free_kernel(uint32_t kernel_handle) {
    (void)kernel_handle;
    /* Would free NPU kernel resources via DRM ioctl */
}

/* ── Runtime Management ── */

int xdna2_runtime_init(xdna2_runtime_t *runtime) {
    if (!runtime) return -1;
    memset(runtime, 0, sizeof(*runtime));

    /* Open NPU device */
    if (xdna2_open_device(&runtime->device_fd) < 0) {
        fprintf(stderr, "xdna2: failed to open NPU device\n");
        return -1;
    }

    /* Query device capabilities */
    xdna2_print_device_info(runtime->device_fd);

    /*
     * Create hardware context with appropriate partition.
     * For a matmul kernel, we need:
     *   - num_tiles: number of AIE cores to allocate
     *   - mem_size: tile memory per core (typically 32KB)
     *   - max_opc: max operations per cycle
     *
     * Default partition may already be configured by the OS.
     * We create a new context to ensure exclusive access.
     */
    xdna2_hwctx_t hwctx;
    int ret = xdna2_create_hwctx(
        runtime->device_fd, &hwctx,
        /* num_tiles */ 16,    /* 16 AIE cores for matmul */
        /* mem_size */ 32768,  /* 32KB per tile */
        /* max_opc */ 4,       /* 4 MACs per tile per cycle */
        /* qos */ AMDXDNA_QOS_HIGH_PRIORITY
    );

    if (ret < 0) {
        fprintf(stderr, "xdna2: failed to create hardware context\n");
        xdna2_close_device(runtime->device_fd);
        return -1;
    }

    /* Store hwctx info in runtime (simplified — real impl needs more state) */
    runtime->initialized = 1;
    runtime->cpu_backend = NULL; /* Set by caller if CPU fallback needed */

    fprintf(stderr, "xdna2: runtime initialized, %d tiles allocated\n", 16);
    return 0;
}

void xdna2_runtime_shutdown(xdna2_runtime_t *runtime) {
    if (!runtime) return;

    if (runtime->initialized) {
        /* Destroy hardware context, close device */
        /* (simplified — real impl needs full cleanup) */
        runtime->initialized = 0;
    }
}

/* ── Matmul Execution ── */

int xdna2_matmul_int8(const float *x, const int8_t *weights,
                       const float *scales, float *y,
                       int S, int I, int O,
                       xdna2_runtime_t *runtime) {
    if (!x || !weights || !y || S <= 0 || I <= 0 || O <= 0) return -1;

    /* Check if NPU kernel exists for this shape */
    uint32_t kernel = xdna2_find_kernel(runtime, S, I, O);

    if (kernel == 0) {
        /* No NPU kernel for this shape — fall back to CPU */
        fprintf(stderr, "xdna2: no NPU kernel for shape (%d, %d, %d), "
                "falling back to CPU\n", S, I, O);

        /* Convert float input to int8 for CPU kernel */
        /* (In production, input would already be quantized) */
        int8_t *x_int8 = (int8_t *)malloc((size_t)S * I * sizeof(int8_t));
        if (!x_int8) return -1;

        for (int s = 0; s < S; s++) {
            for (int i = 0; i < I; i++) {
                float val = x[s * I + i];
                x_int8[s * I + i] = (int8_t)roundf(val);
            }
        }

        strix_cpu_matmul(x_int8, S, I, weights, O, y, scales);
        free(x_int8);
        return 0;
    }

    /*
     * NPU path (requires aiecompiler and compiled kernels):
     *
     * 1. Allocate BOs for input, weights, output
     * 2. Sync data to device (DMA)
     * 3. Submit exec command with kernel args
     * 4. Wait for completion
     * 5. Sync results back
     *
     * This path is not yet implemented — aiecompiler is required.
     */
    fprintf(stderr, "xdna2: NPU kernel %u found but execution not yet "
            "implemented (requires aiecompiler)\n", kernel);

    /* Fallback to CPU */
    int8_t *x_int8 = (int8_t *)malloc((size_t)S * I * sizeof(int8_t));
    if (!x_int8) return -1;

    for (int s = 0; s < S; s++) {
        for (int i = 0; i < I; i++) {
            float val = x[s * I + i];
            x_int8[s * I + i] = (int8_t)roundf(val);
        }
    }

    strix_cpu_matmul(x_int8, S, I, weights, O, y, scales);
    free(x_int8);
    return 0;
}

int xdna2_matmul_int4(const float *x, const uint8_t *weights_packed,
                       const float *scales, float *y,
                       int S, int I, int O,
                       xdna2_runtime_t *runtime) {
    if (!x || !weights_packed || !y || S <= 0 || I <= 0 || O <= 0) return -1;

    /* Int4 weights need dequantization to int8 before NPU execution.
     * The NPU only supports int8 MAC operations. */

    /* Dequantize int4 -> int8 */
    int8_t *weights_i8 = (int8_t *)malloc((size_t)O * I * sizeof(int8_t));
    if (!weights_i8) return -1;

    for (int o = 0; o < O; o++) {
        for (int i = 0; i < I; i++) {
            int nibble = (i & 1) ?
                (weights_packed[o * ((I + 1) / 2) + (i >> 1)] >> 4) :
                (weights_packed[o * ((I + 1) / 2) + (i >> 1)] & 0x0f);
            weights_i8[o * I + i] = (int8_t)(nibble - 8); /* sign-extend */
        }
    }

    /* Execute int8 matmul */
    int ret = xdna2_matmul_int8(x, weights_i8, scales, y, S, I, O, runtime);

    free(weights_i8);
    return ret;
}

/* ── Shape Registration ── */

/* Simple shape table — in production, use a hash map */
#define MAX_REGISTERED_SHAPES 32

typedef struct {
    xdna2_matmul_shape_t shape;
    uint32_t kernel_handle;
} shape_entry_t;

static shape_entry_t g_shape_table[MAX_REGISTERED_SHAPES];
static int g_shape_count = 0;

int xdna2_register_shape(xdna2_runtime_t *runtime,
                          const xdna2_matmul_shape_t *shape,
                          uint32_t kernel_handle) {
    (void)runtime;

    if (!shape || g_shape_count >= MAX_REGISTERED_SHAPES) return -1;

    g_shape_table[g_shape_count].shape = *shape;
    g_shape_table[g_shape_count].kernel_handle = kernel_handle;
    g_shape_count++;

    fprintf(stderr, "xdna2: registered shape (%d, %d, %d) -> kernel %u\n",
            shape->rows, shape->inner_dim, shape->out_cols, kernel_handle);
    return 0;
}

uint32_t xdna2_find_kernel(xdna2_runtime_t *runtime,
                            int rows, int inner_dim, int out_cols) {
    (void)runtime;

    for (int i = 0; i < g_shape_count; i++) {
        if (g_shape_table[i].shape.rows == rows &&
            g_shape_table[i].shape.inner_dim == inner_dim &&
            g_shape_table[i].shape.out_cols == out_cols) {
            return g_shape_table[i].kernel_handle;
        }
    }

    return 0; /* No matching kernel */
}