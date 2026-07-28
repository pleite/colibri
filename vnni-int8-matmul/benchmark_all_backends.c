/*
 * benchmark_all_backends.c — measure backend execution for placement and
 * batching experiments on Strix Halo.
 *
 * The tool exercises the actual CPU/Vulkan/NPU backends with a small sweep of:
 *   - batch size (how many matmuls are issued per measurement window)
 *   - thread count (CPU worker count for the batch loop)
 *   - matrix shape (the same shapes used by the placement policy benchmark)
 *
 * Each row records the elapsed time for a measured batch plus an estimate of the
 * memory traffic that the batch would imply, so the resulting CSV can answer
 * "where should this tensor live?" and "what batching/threading point is worth
 * it?" on the device that is being tuned.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "cpu/vnni_cpu_backend.h"
#include "gpu/vulkan_backend.h"
#include "npu/xdna2_backend.h"
#include "c/npu_kernels/xdna2_matmul.h"

#ifndef COLI_HAVE_VULKAN
static int benchmark_gpu_stub_is_supported(void) { return 0; }
static const char *benchmark_gpu_stub_backend_name(void) { return "vulkan-unavailable"; }
static const char *benchmark_gpu_stub_failure_reason(void) { return "vulkan headers not available"; }
static int benchmark_gpu_stub_matmul(const int8_t *input, int rows, int inner_dim,
                                     const int8_t *weights, int out_cols,
                                     float *output, const float *scales) {
    (void)input; (void)rows; (void)inner_dim; (void)weights; (void)out_cols;
    (void)output; (void)scales;
    return 0;
}
static int benchmark_gpu_stub_batch_matmul(const int8_t *input, int rows, int inner_dim,
                                           const int8_t *weights, int out_cols,
                                           float *output, const float *scales, int batch_size) {
    (void)input; (void)rows; (void)inner_dim; (void)weights; (void)out_cols;
    (void)output; (void)scales; (void)batch_size;
    return 0;
}
static void benchmark_gpu_stub_shutdown(void) {}
#define strix_vulkan_is_supported benchmark_gpu_stub_is_supported
#define strix_vulkan_backend_name benchmark_gpu_stub_backend_name
#define strix_vulkan_failure_reason benchmark_gpu_stub_failure_reason
#define strix_vulkan_matmul benchmark_gpu_stub_matmul
#define strix_vulkan_batch_matmul benchmark_gpu_stub_batch_matmul
#define strix_vulkan_shutdown benchmark_gpu_stub_shutdown
#endif

#ifndef COLI_HAVE_NPU
static int benchmark_npu_stub_is_supported(void) { return 0; }
static const char *benchmark_npu_stub_backend_name(void) { return "xdna2-unavailable"; }
static const char *benchmark_npu_stub_failure_reason(void) { return "amdxdna UAPI headers not available"; }
static int benchmark_npu_stub_kernel_exists(int rows, int inner_dim, int out_cols, int fmt) {
    (void)rows; (void)inner_dim; (void)out_cols; (void)fmt; return 0;
}
static int benchmark_npu_stub_matmul(const int8_t *input, int rows, int inner_dim,
                                     const int8_t *weights, int out_cols,
                                     float *output, const float *scales) {
    (void)input; (void)rows; (void)inner_dim; (void)weights; (void)out_cols;
    (void)output; (void)scales; return 0;
}
static int benchmark_npu_stub_batch_matmul(const int8_t *input, int rows, int inner_dim,
                                           const int8_t *weights, int out_cols,
                                           float *output, const float *scales, int batch_size) {
    (void)input; (void)rows; (void)inner_dim; (void)weights; (void)out_cols;
    (void)output; (void)scales; (void)batch_size; return 0;
}
static void benchmark_npu_stub_shutdown(void) {}
#define strix_xdna2_is_supported benchmark_npu_stub_is_supported
#define strix_xdna2_backend_name benchmark_npu_stub_backend_name
#define strix_xdna2_failure_reason benchmark_npu_stub_failure_reason
#define strix_xdna2_kernel_exists benchmark_npu_stub_kernel_exists
#define strix_xdna2_matmul benchmark_npu_stub_matmul
#define strix_xdna2_batch_matmul benchmark_npu_stub_batch_matmul
#define strix_xdna2_shutdown benchmark_npu_stub_shutdown
#endif

#define DEFAULT_ITERS 5
#define DEFAULT_BATCH 1
#define DEFAULT_THREADS 1

typedef struct {
    int rows;
    int inner_dim;
    int out_cols;
} workload_shape_t;

typedef struct {
    int thread_id;
    int thread_count;
    int batch_size;
    int rows;
    int inner_dim;
    int out_cols;
    const int8_t *input;
    const int8_t *weights;
    const float *scales;
    float *output;
    int success;
} cpu_worker_args_t;

typedef struct {
    const char *name;
    int (*is_supported)(void);
    const char *(*backend_name)(void);
    const char *(*failure_reason)(void);
} backend_entry_t;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int total_count;
    int count;
    int generation;
} benchmark_barrier_t;

typedef struct {
    int backend_kind;
    int batch_size;
    int threads;
    int iters;
    int rows;
    int inner_dim;
    int out_cols;
    int batching_mode;
    benchmark_barrier_t *barrier;
    double elapsed_ms;
    double bytes_processed;
    int success;
    const char *name;
} concurrent_worker_args_t;

static int benchmark_barrier_init(benchmark_barrier_t *barrier, int count) {
    if (!barrier || count <= 0) return 0;
    if (pthread_mutex_init(&barrier->mutex, NULL) != 0) return 0;
    if (pthread_cond_init(&barrier->cond, NULL) != 0) {
        pthread_mutex_destroy(&barrier->mutex);
        return 0;
    }
    barrier->total_count = count;
    barrier->count = count;
    barrier->generation = 0;
    return 1;
}

static void benchmark_barrier_destroy(benchmark_barrier_t *barrier) {
    if (!barrier) return;
    pthread_cond_destroy(&barrier->cond);
    pthread_mutex_destroy(&barrier->mutex);
}

static int benchmark_barrier_wait(benchmark_barrier_t *barrier) {
    if (!barrier) return 0;
    pthread_mutex_lock(&barrier->mutex);
    int generation = barrier->generation;
    barrier->count--;
    if (barrier->count == 0) {
        barrier->generation++;
        barrier->count = barrier->total_count;
        pthread_cond_broadcast(&barrier->cond);
    } else {
        while (barrier->generation == generation) {
            pthread_cond_wait(&barrier->cond, &barrier->mutex);
        }
    }
    pthread_mutex_unlock(&barrier->mutex);
    return 1;
}

static const workload_shape_t g_shapes[] = {
    {256, 4096, 1024},
    {256, 1024, 4096},
    {256, 4096, 16384},
    {256, 4096, 512},
    {256, 8192, 4096},
    {32, 4096, 1024},
    {32, 1024, 4096},
    {32, 4096, 16384},
    {32, 4096, 512},
    {32, 8192, 4096},
    {1, 4096, 1024},
    {1, 1024, 4096},
    {1, 4096, 512},
    {8, 4096, 1024},
    {64, 4096, 1024},
};

static const size_t g_shape_count = sizeof(g_shapes) / sizeof(g_shapes[0]);

static uint64_t now_ns(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
    return ((uint64_t)ts.tv_sec * 1000000000ull) + (uint64_t)ts.tv_nsec;
}

static int env_int(const char *name, int fallback) {
    const char *value = getenv(name);
    if (!value || !*value) return fallback;
    char *end = NULL;
    errno = 0;
    long parsed = strtol(value, &end, 10);
    if (errno != 0 || !end || end == value || *end != '\0' || parsed <= 0) {
        return fallback;
    }
    return (int)parsed;
}

static int parse_backend(const char *value) {
    if (!value || !*value) return -1;
    if (strcmp(value, "cpu") == 0) return 0;
    if (strcmp(value, "gpu") == 0) return 1;
    if (strcmp(value, "npu") == 0) return 2;
    if (strcmp(value, "all") == 0) return 3;
    return -1;
}

static void *cpu_worker_entry(void *arg) {
    cpu_worker_args_t *ctx = (cpu_worker_args_t *)arg;
    const size_t stride = (size_t)ctx->rows * (size_t)ctx->out_cols;
    for (int task = ctx->thread_id; task < ctx->batch_size; task += ctx->thread_count) {
        const size_t offset = (size_t)task * stride;
        if (!strix_cpu_matmul(ctx->input, ctx->rows, ctx->inner_dim, ctx->weights,
                              ctx->out_cols, ctx->output + offset, ctx->scales)) {
            ctx->success = 0;
            return NULL;
        }
    }
    return NULL;
}

static int run_backend_benchmark(int backend_kind, const workload_shape_t *shape,
                                 int batch_size, int threads, int iters,
                                 int batching_mode, double *elapsed_ms,
                                 double *bytes_processed) {
    if (backend_kind == 0) {
        return run_cpu_benchmark(shape, batch_size, threads, iters, elapsed_ms, bytes_processed);
    }
    if (backend_kind == 1) {
        if (batching_mode) {
            return run_gpu_benchmark_batched(shape, batch_size, iters, elapsed_ms, bytes_processed);
        }
        return run_gpu_benchmark(shape, batch_size, iters, elapsed_ms, bytes_processed);
    }
    if (backend_kind == 2) {
        if (batching_mode) {
            return run_npu_benchmark_batched(shape, batch_size, iters, elapsed_ms, bytes_processed);
        }
        return run_npu_benchmark(shape, batch_size, iters, elapsed_ms, bytes_processed);
    }
    return 0;
}

static int run_cpu_benchmark(const workload_shape_t *shape, int batch_size, int threads,
                             int iters, double *elapsed_ms, double *bytes_processed) {
    int8_t *input = NULL;
    int8_t *weights = NULL;
    float *scales = NULL;
    float *output = NULL;
    int success = 0;

    const size_t input_bytes = (size_t)shape->rows * (size_t)shape->inner_dim;
    const size_t weight_bytes = (size_t)shape->out_cols * (size_t)shape->inner_dim;
    const size_t output_bytes = (size_t)shape->rows * (size_t)shape->out_cols * sizeof(float);
    const size_t scale_bytes = (size_t)shape->out_cols * sizeof(float);
    const size_t bytes_per_call = input_bytes + weight_bytes + output_bytes + scale_bytes;

    input = (int8_t *)malloc(input_bytes);
    weights = (int8_t *)malloc(weight_bytes);
    scales = (float *)malloc(scale_bytes);
    output = (float *)malloc((size_t)batch_size * (size_t)shape->rows * (size_t)shape->out_cols * sizeof(float));
    if (!input || !weights || !scales || !output) {
        goto out;
    }

    for (size_t i = 0; i < input_bytes; ++i) {
        input[i] = (int8_t)((int)(i * 3 + 1) % 17 - 8);
    }
    for (size_t i = 0; i < weight_bytes; ++i) {
        weights[i] = (int8_t)((int)(i * 5 + 3) % 13 - 6);
    }
    for (int i = 0; i < shape->out_cols; ++i) {
        scales[i] = 1.0f / 128.0f;
    }
    memset(output, 0, (size_t)batch_size * (size_t)shape->rows * (size_t)shape->out_cols * sizeof(float));

    /* Warm-up once so the measurement is not dominated by first-touch faults. */
    if (!strix_cpu_matmul(input, shape->rows, shape->inner_dim, weights,
                          shape->out_cols, output, scales)) {
        goto out;
    }

    uint64_t total_ns = 0;
    const int worker_count = (threads > 1) ? threads : 1;
    for (int i = 0; i < iters; ++i) {
        cpu_worker_args_t worker_args;
        pthread_t *thread_handles = NULL;
        cpu_worker_args_t *workers = NULL;
        int local_success = 1;

        memset(&worker_args, 0, sizeof(worker_args));
        worker_args.thread_id = 0;
        worker_args.thread_count = worker_count;
        worker_args.batch_size = batch_size;
        worker_args.rows = shape->rows;
        worker_args.inner_dim = shape->inner_dim;
        worker_args.out_cols = shape->out_cols;
        worker_args.input = input;
        worker_args.weights = weights;
        worker_args.scales = scales;
        worker_args.output = output;
        worker_args.success = 1;

        const uint64_t start = now_ns();
        if (worker_count > 1) {
            thread_handles = (pthread_t *)calloc((size_t)worker_count, sizeof(*thread_handles));
            workers = (cpu_worker_args_t *)calloc((size_t)worker_count, sizeof(*workers));
            if (!thread_handles || !workers) {
                local_success = 0;
            } else {
                for (int t = 0; t < worker_count; ++t) {
                    workers[t] = worker_args;
                    workers[t].thread_id = t;
                    workers[t].thread_count = worker_count;
                    if (pthread_create(&thread_handles[t], NULL, cpu_worker_entry, &workers[t]) != 0) {
                        local_success = 0;
                        for (int j = 0; j < t; ++j) pthread_join(thread_handles[j], NULL);
                        break;
                    }
                }
                if (local_success) {
                    for (int t = 0; t < worker_count; ++t) {
                        pthread_join(thread_handles[t], NULL);
                    }
                }
            }
        } else {
            cpu_worker_args_t single_worker = worker_args;
            single_worker.thread_id = 0;
            single_worker.thread_count = 1;
            cpu_worker_entry(&single_worker);
            worker_args.success = single_worker.success;
        }

        if (worker_count > 1) {
            for (int t = 0; t < worker_count; ++t) {
                if (!workers[t].success) worker_args.success = 0;
            }
            free(thread_handles);
            free(workers);
        }

        if (!local_success || !worker_args.success) goto out;

        const uint64_t elapsed = now_ns() - start;
        total_ns += elapsed;
    }

    *elapsed_ms = (double)total_ns / (double)iters / 1000000.0;
    *bytes_processed = (double)iters * (double)batch_size * (double)bytes_per_call;
    success = 1;

out:
    free(input);
    free(weights);
    free(scales);
    free(output);
    return success;
}

static int run_gpu_benchmark(const workload_shape_t *shape, int batch_size, int iters,
                             double *elapsed_ms, double *bytes_processed) {
    int8_t *input = NULL;
    int8_t *weights = NULL;
    float *scales = NULL;
    float *output = NULL;
    int success = 0;

    const size_t input_bytes = (size_t)shape->rows * (size_t)shape->inner_dim;
    const size_t weight_bytes = (size_t)shape->out_cols * (size_t)shape->inner_dim;
    const size_t output_bytes = (size_t)shape->rows * (size_t)shape->out_cols * sizeof(float);
    const size_t scale_bytes = (size_t)shape->out_cols * sizeof(float);
    const size_t bytes_per_call = input_bytes + weight_bytes + output_bytes + scale_bytes;

    input = (int8_t *)malloc(input_bytes);
    weights = (int8_t *)malloc(weight_bytes);
    scales = (float *)malloc(scale_bytes);
    output = (float *)malloc((size_t)shape->rows * (size_t)shape->out_cols * sizeof(float));
    if (!input || !weights || !scales || !output) goto out;

    for (size_t i = 0; i < input_bytes; ++i) input[i] = (int8_t)((int)(i * 3 + 1) % 17 - 8);
    for (size_t i = 0; i < weight_bytes; ++i) weights[i] = (int8_t)((int)(i * 5 + 3) % 13 - 6);
    for (int i = 0; i < shape->out_cols; ++i) scales[i] = 1.0f / 128.0f;
    memset(output, 0, (size_t)shape->rows * (size_t)shape->out_cols * sizeof(float));

    if (!strix_vulkan_matmul(input, shape->rows, shape->inner_dim, weights,
                             shape->out_cols, output, scales)) {
        goto out;
    }

    uint64_t total_ns = 0;
    for (int i = 0; i < iters; ++i) {
        const uint64_t start = now_ns();
        for (int b = 0; b < batch_size; ++b) {
            if (!strix_vulkan_matmul(input, shape->rows, shape->inner_dim, weights,
                                     shape->out_cols, output, scales)) {
                goto out;
            }
        }
        total_ns += now_ns() - start;
    }

    *elapsed_ms = (double)total_ns / (double)iters / 1000000.0;
    *bytes_processed = (double)iters * (double)batch_size * (double)bytes_per_call;
    success = 1;

out:
    free(input);
    free(weights);
    free(scales);
    free(output);
    return success;
}

/*
 * run_gpu_benchmark_batched — GPU benchmark using strix_vulkan_batch_matmul().
 *
 * Matrices are pre-allocated once outside the iteration loop (Phase 1 memory
 * optimisation).  All `batch_size` matmuls share the same input and weight
 * buffers and are submitted in a single vkQueueSubmit() followed by one
 * fence wait (Phase 2 command batching).
 */
static int run_gpu_benchmark_batched(const workload_shape_t *shape, int batch_size, int iters,
                                     double *elapsed_ms, double *bytes_processed) {
    int8_t *input = NULL;
    int8_t *weights = NULL;
    float *scales = NULL;
    float *output = NULL;
    int success = 0;

    const size_t input_bytes = (size_t)shape->rows * (size_t)shape->inner_dim;
    const size_t weight_bytes = (size_t)shape->out_cols * (size_t)shape->inner_dim;
    const size_t output_bytes = (size_t)shape->rows * (size_t)shape->out_cols * sizeof(float);
    const size_t scale_bytes = (size_t)shape->out_cols * sizeof(float);
    const size_t bytes_per_call = input_bytes + weight_bytes + output_bytes + scale_bytes;

    /* Preallocate and initialise once — reused across all iters and batch items. */
    input = (int8_t *)malloc(input_bytes);
    weights = (int8_t *)malloc(weight_bytes);
    scales = (float *)malloc(scale_bytes);
    /* Output holds all batch_size slices contiguously. */
    output = (float *)malloc((size_t)batch_size * (size_t)shape->rows *
                             (size_t)shape->out_cols * sizeof(float));
    if (!input || !weights || !scales || !output) goto out;

    for (size_t i = 0; i < input_bytes; ++i) input[i] = (int8_t)((int)(i * 3 + 1) % 17 - 8);
    for (size_t i = 0; i < weight_bytes; ++i) weights[i] = (int8_t)((int)(i * 5 + 3) % 13 - 6);
    for (int i = 0; i < shape->out_cols; ++i) scales[i] = 1.0f / 128.0f;
    memset(output, 0, (size_t)batch_size * (size_t)shape->rows *
           (size_t)shape->out_cols * sizeof(float));

    /* Warm-up outside the timed loop. */
    if (!strix_vulkan_batch_matmul(input, shape->rows, shape->inner_dim, weights,
                                   shape->out_cols, output, scales, batch_size)) {
        goto out;
    }

    uint64_t total_ns = 0;
    for (int i = 0; i < iters; ++i) {
        const uint64_t start = now_ns();
        if (!strix_vulkan_batch_matmul(input, shape->rows, shape->inner_dim, weights,
                                       shape->out_cols, output, scales, batch_size)) {
            goto out;
        }
        total_ns += now_ns() - start;
    }

    *elapsed_ms = (double)total_ns / (double)iters / 1000000.0;
    *bytes_processed = (double)iters * (double)batch_size * (double)bytes_per_call;
    success = 1;

out:
    free(input);
    free(weights);
    free(scales);
    free(output);
    return success;
}

static int run_npu_benchmark(const workload_shape_t *shape, int batch_size, int iters,
                             double *elapsed_ms, double *bytes_processed) {
    int8_t *input = NULL;
    int8_t *weights = NULL;
    float *scales = NULL;
    float *output = NULL;
    int success = 0;

    const size_t input_bytes = (size_t)shape->rows * (size_t)shape->inner_dim;
    const size_t weight_bytes = (size_t)shape->out_cols * (size_t)shape->inner_dim;
    const size_t output_bytes = (size_t)shape->rows * (size_t)shape->out_cols * sizeof(float);
    const size_t scale_bytes = (size_t)shape->out_cols * sizeof(float);
    const size_t bytes_per_call = input_bytes + weight_bytes + output_bytes + scale_bytes;

    input = (int8_t *)malloc(input_bytes);
    weights = (int8_t *)malloc(weight_bytes);
    scales = (float *)malloc(scale_bytes);
    output = (float *)malloc((size_t)shape->rows * (size_t)shape->out_cols * sizeof(float));
    if (!input || !weights || !scales || !output) goto out;

    for (size_t i = 0; i < input_bytes; ++i) input[i] = (int8_t)((int)(i * 3 + 1) % 17 - 8);
    for (size_t i = 0; i < weight_bytes; ++i) weights[i] = (int8_t)((int)(i * 5 + 3) % 13 - 6);
    for (int i = 0; i < shape->out_cols; ++i) scales[i] = 1.0f / 128.0f;
    memset(output, 0, (size_t)shape->rows * (size_t)shape->out_cols * sizeof(float));

    if (!strix_xdna2_kernel_exists(shape->rows, shape->inner_dim, shape->out_cols,
                                   XDNA2_FMT_INT8)) {
        goto out;
    }
    if (!strix_xdna2_matmul(input, shape->rows, shape->inner_dim, weights,
                            shape->out_cols, output, scales)) {
        goto out;
    }

    uint64_t total_ns = 0;
    for (int i = 0; i < iters; ++i) {
        const uint64_t start = now_ns();
        for (int b = 0; b < batch_size; ++b) {
            if (!strix_xdna2_matmul(input, shape->rows, shape->inner_dim, weights,
                                    shape->out_cols, output, scales)) {
                goto out;
            }
        }
        total_ns += now_ns() - start;
    }

    *elapsed_ms = (double)total_ns / (double)iters / 1000000.0;
    *bytes_processed = (double)iters * (double)batch_size * (double)bytes_per_call;
    success = 1;

out:
    free(input);
    free(weights);
    free(scales);
    free(output);
    return success;
}

/*
 * run_npu_benchmark_batched — NPU benchmark using strix_xdna2_batch_matmul().
 *
 * Matrices are pre-allocated once outside the iteration loop (Phase 1 memory
 * optimisation).  All `batch_size` commands share the same input and weight
 * BOs; they are submitted without an intervening wait and the single timeline-
 * syncobj wait covers the whole batch (Phase 3 async dispatch).
 */
static int run_npu_benchmark_batched(const workload_shape_t *shape, int batch_size, int iters,
                                     double *elapsed_ms, double *bytes_processed) {
    int8_t *input = NULL;
    int8_t *weights = NULL;
    float *scales = NULL;
    float *output = NULL;
    int success = 0;

    const size_t input_bytes = (size_t)shape->rows * (size_t)shape->inner_dim;
    const size_t weight_bytes = (size_t)shape->out_cols * (size_t)shape->inner_dim;
    const size_t output_bytes = (size_t)shape->rows * (size_t)shape->out_cols * sizeof(float);
    const size_t scale_bytes = (size_t)shape->out_cols * sizeof(float);
    const size_t bytes_per_call = input_bytes + weight_bytes + output_bytes + scale_bytes;

    /* Preallocate and initialise once — reused across all iters. */
    input = (int8_t *)malloc(input_bytes);
    weights = (int8_t *)malloc(weight_bytes);
    scales = (float *)malloc(scale_bytes);
    /* Output holds all batch_size slices contiguously. */
    output = (float *)malloc((size_t)batch_size * (size_t)shape->rows *
                             (size_t)shape->out_cols * sizeof(float));
    if (!input || !weights || !scales || !output) goto out;

    for (size_t i = 0; i < input_bytes; ++i) input[i] = (int8_t)((int)(i * 3 + 1) % 17 - 8);
    for (size_t i = 0; i < weight_bytes; ++i) weights[i] = (int8_t)((int)(i * 5 + 3) % 13 - 6);
    for (int i = 0; i < shape->out_cols; ++i) scales[i] = 1.0f / 128.0f;
    memset(output, 0, (size_t)batch_size * (size_t)shape->rows *
           (size_t)shape->out_cols * sizeof(float));

    if (!strix_xdna2_kernel_exists(shape->rows, shape->inner_dim, shape->out_cols,
                                   XDNA2_FMT_INT8)) {
        goto out;
    }

    /* Warm-up outside the timed loop. */
    if (!strix_xdna2_batch_matmul(input, shape->rows, shape->inner_dim, weights,
                                  shape->out_cols, output, scales, batch_size)) {
        goto out;
    }

    uint64_t total_ns = 0;
    for (int i = 0; i < iters; ++i) {
        const uint64_t start = now_ns();
        if (!strix_xdna2_batch_matmul(input, shape->rows, shape->inner_dim, weights,
                                      shape->out_cols, output, scales, batch_size)) {
            goto out;
        }
        total_ns += now_ns() - start;
    }

    *elapsed_ms = (double)total_ns / (double)iters / 1000000.0;
    *bytes_processed = (double)iters * (double)batch_size * (double)bytes_per_call;
    success = 1;

out:
    free(input);
    free(weights);
    free(scales);
    free(output);
    return success;
}

static void print_usage(const char *argv0) {
    fprintf(stderr,
            "Usage: %s [--backend cpu|gpu|npu|all] [--batch N] [--threads N] [--iters N]\n"
            "          [--csv FILE] [--batching-mode serial|batched]\n"
            "          [--concurrent] [--mixed]\n",
            argv0);
}

static void write_csv_row(FILE *csv, const char *backend, const workload_shape_t *shape,
                          int batch_size, int threads, int iters, double elapsed_ms,
                          int success, double bytes_processed, const char *mode,
                          const char *active_backends) {
    const double elapsed_s = elapsed_ms / 1000.0;
    const double estimated_gib_s = elapsed_s > 0.0
        ? bytes_processed / (elapsed_s * 1024.0 * 1024.0 * 1024.0)
        : 0.0;
    fprintf(csv, "%s,%d,%d,%d,%d,%d,%d,%.6f,%d,%.0f,%.6f,%s,%s\n",
            backend, shape->rows, shape->inner_dim, shape->out_cols,
            batch_size, threads, iters, elapsed_ms, success,
            bytes_processed, estimated_gib_s, mode, active_backends ? active_backends : "none");
    fflush(csv);
}

static void *concurrent_backend_worker(void *arg) {
    concurrent_worker_args_t *ctx = (concurrent_worker_args_t *)arg;
    if (!ctx->barrier) {
        ctx->success = 0;
        return NULL;
    }
    if (!benchmark_barrier_wait(ctx->barrier)) {
        ctx->success = 0;
        return NULL;
    }

    double bench_elapsed_ms = 0.0;
    double bench_bytes_processed = 0.0;
    ctx->success = run_backend_benchmark(ctx->backend_kind,
                                         &(workload_shape_t){ctx->rows, ctx->inner_dim, ctx->out_cols},
                                         ctx->batch_size, ctx->threads, ctx->iters,
                                         ctx->batching_mode, &bench_elapsed_ms,
                                         &bench_bytes_processed);
    ctx->elapsed_ms = bench_elapsed_ms;
    ctx->bytes_processed = bench_bytes_processed;
    return NULL;
}

int main(int argc, char **argv) {
    const char *backend_filter = "all";
    int batch_size = DEFAULT_BATCH;
    int threads = DEFAULT_THREADS;
    int iters = env_int("COLI_BENCH_ITERS", DEFAULT_ITERS);
    const char *csv_path = "benchmark_results.csv";
    /* batching_mode: 0 = serial (default, backward compatible), 1 = batched */
    int batching_mode = 0;
    int concurrent_mode = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--backend") == 0 && i + 1 < argc) {
            backend_filter = argv[++i];
        } else if (strcmp(argv[i], "--batch") == 0 && i + 1 < argc) {
            batch_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iters") == 0 && i + 1 < argc) {
            iters = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--csv") == 0 && i + 1 < argc) {
            csv_path = argv[++i];
        } else if (strcmp(argv[i], "--batching-mode") == 0 && i + 1 < argc) {
            const char *mode = argv[++i];
            if (strcmp(mode, "batched") == 0) {
                batching_mode = 1;
            } else if (strcmp(mode, "serial") == 0) {
                batching_mode = 0;
            } else {
                fprintf(stderr, "benchmark_all_backends: unknown batching-mode '%s' "
                        "(use serial or batched)\n", mode);
                return 2;
            }
        } else if (strcmp(argv[i], "--concurrent") == 0 || strcmp(argv[i], "--mixed") == 0) {
            concurrent_mode = 1;
        } else if (strcmp(argv[i], "--duration") == 0 && i + 1 < argc) {
            /* Keep the older script's flag compatible; it is an alias for a
             * small iteration count when the user just wants a fast sweep. */
            iters = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            print_usage(argv[0]);
            return 2;
        }
    }

    if (batch_size <= 0) batch_size = DEFAULT_BATCH;
    if (threads <= 0) threads = DEFAULT_THREADS;
    if (iters <= 0) iters = DEFAULT_ITERS;

    FILE *csv = fopen(csv_path, "w");
    if (!csv) {
        fprintf(stderr, "benchmark_all_backends: cannot open %s for writing: %s\n",
                csv_path, strerror(errno));
        return 1;
    }

    fprintf(csv, "backend,rows,inner_dim,out_cols,batch_size,threads,iters,elapsed_ms,success,bytes_processed,estimated_gib_s,mode,active_backends\n");

    const int backend_kind = parse_backend(backend_filter);
    const backend_entry_t entries[] = {
        {"cpu", strix_cpu_is_supported, strix_cpu_backend_name, strix_cpu_backend_name},
        {"gpu", strix_vulkan_is_supported, strix_vulkan_backend_name, strix_vulkan_failure_reason},
        {"npu", strix_xdna2_is_supported, strix_xdna2_backend_name, strix_xdna2_failure_reason},
    };

    const char *run_mode = batching_mode ? "batched" : "serial";

    for (size_t s = 0; s < g_shape_count; ++s) {
        const workload_shape_t *shape = &g_shapes[s];

        if (concurrent_mode) {
            int selected_kinds[3];
            char combined_names[64] = "";
            int selected_count = 0;

            for (size_t k = 0; k < sizeof(entries) / sizeof(entries[0]); ++k) {
                const char *name = entries[k].name;
                if (backend_kind != 3 && backend_kind != (int)k) {
                   continue;
                }
                if (!entries[k].is_supported()) {
                   printf("SKIP %s: %s\n", name, entries[k].failure_reason());
                   continue;
                }
                if (selected_count > 0) {
                   strncat(combined_names, ",", sizeof(combined_names) - strlen(combined_names) - 1);
                }
                strncat(combined_names, name, sizeof(combined_names) - strlen(combined_names) - 1);
                selected_kinds[selected_count++] = (int)k;
            }

            if (selected_count == 0) {
                continue;
            }

            benchmark_barrier_t barrier;
            if (!benchmark_barrier_init(&barrier, selected_count + 1)) {
                fprintf(stderr, "benchmark_all_backends: could not initialize barrier\n");
                fclose(csv);
                return 1;
            }

            concurrent_worker_args_t *workers = (concurrent_worker_args_t *)calloc(
                (size_t)selected_count, sizeof(*workers));
            pthread_t *worker_threads = (pthread_t *)calloc((size_t)selected_count, sizeof(*worker_threads));
            if (!workers || !worker_threads) {
                free(workers);
                free(worker_threads);
                benchmark_barrier_destroy(&barrier);
                fclose(csv);
                return 1;
            }

            for (int idx = 0; idx < selected_count; ++idx) {
                const int backend_idx = selected_kinds[idx];
                workers[idx].backend_kind = backend_idx;
                workers[idx].batch_size = batch_size;
                workers[idx].threads = (backend_idx == 0) ? threads : 1;
                workers[idx].iters = iters;
                workers[idx].rows = shape->rows;
                workers[idx].inner_dim = shape->inner_dim;
                workers[idx].out_cols = shape->out_cols;
                workers[idx].batching_mode = batching_mode;
                workers[idx].barrier = &barrier;
                workers[idx].success = 0;
                workers[idx].elapsed_ms = 0.0;
                workers[idx].bytes_processed = 0.0;
                workers[idx].name = entries[backend_idx].name;
                if (pthread_create(&worker_threads[idx], NULL, concurrent_backend_worker, &workers[idx]) != 0) {
                   fprintf(stderr, "benchmark_all_backends: failed to create %s worker thread\n", entries[backend_idx].name);
                   for (int j = 0; j < idx; ++j) pthread_join(worker_threads[j], NULL);
                   free(workers);
                   free(worker_threads);
                   benchmark_barrier_destroy(&barrier);
                   fclose(csv);
                   return 1;
                }
            }

            printf("running mixed rows=%d inner=%d out=%d batch=%d threads=%d iters=%d backends=%s\n",
                   shape->rows, shape->inner_dim, shape->out_cols, batch_size,
                   threads, iters, combined_names);

            const uint64_t overall_start = now_ns();
            benchmark_barrier_wait(&barrier);
            for (int idx = 0; idx < selected_count; ++idx) {
                pthread_join(worker_threads[idx], NULL);
            }
            const uint64_t overall_end = now_ns();
            const double combined_elapsed_ms = (double)(overall_end - overall_start) / 1000000.0;

            for (int idx = 0; idx < selected_count; ++idx) {
                write_csv_row(csv, workers[idx].name, shape, batch_size,
                             workers[idx].threads, iters, workers[idx].elapsed_ms,
                             workers[idx].success, workers[idx].bytes_processed,
                             "concurrent", combined_names);
            }
            printf("mixed summary rows=%d inner=%d out=%d elapsed_ms=%.3f backends=%s\n",
                   shape->rows, shape->inner_dim, shape->out_cols,
                   combined_elapsed_ms, combined_names);

            free(workers);
            free(worker_threads);
            benchmark_barrier_destroy(&barrier);
            continue;
        }

        for (size_t k = 0; k < sizeof(entries) / sizeof(entries[0]); ++k) {
            const char *name = entries[k].name;
            if (backend_kind != 3 && backend_kind != (int)k) {
                continue;
            }

            if (!entries[k].is_supported()) {
                printf("SKIP %s: %s\n", name, entries[k].failure_reason());
                continue;
            }

            double elapsed_ms = 0.0;
            double bytes_processed = 0.0;
            int success = 0;

            printf("running %s rows=%d inner=%d out=%d batch=%d threads=%d iters=%d mode=%s\n",
                   name, shape->rows, shape->inner_dim, shape->out_cols,
                   batch_size, (strcmp(name, "cpu") == 0) ? threads : 1, iters,
                   run_mode);

            success = run_backend_benchmark((int)k, shape, batch_size, (strcmp(name, "cpu") == 0) ? threads : 1,
                                           iters, batching_mode, &elapsed_ms, &bytes_processed);

            write_csv_row(csv, name, shape, batch_size,
                        (strcmp(name, "cpu") == 0) ? threads : 1, iters,
                        elapsed_ms, success, bytes_processed,
                        run_mode, "single");
        }
    }

    fclose(csv);

    strix_vulkan_shutdown();
    strix_xdna2_shutdown();
    return 0;
}
