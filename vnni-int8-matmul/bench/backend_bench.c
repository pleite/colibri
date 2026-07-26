/**
 * backend_bench.c — measure int8 matmul cost per engine on the Strix Halo
 * machine, and emit the table the placement policy consults.
 *
 * This exists because `backend_placement.c` refuses to contain timing
 * constants. The sweep covers the three regimes that actually change the
 * answer:
 *
 *   rows = 1            decode: one token, no weight reuse at all
 *   rows = 8, 32, 64    small prefill / a busy MoE expert group
 *   rows = 256, 512     bulk prefill
 *
 * crossed with the Qwen 3.6 MoE projections enumerated in sched/npu_shapes.h,
 * so the measured shapes are the shapes the model asks for rather than round
 * numbers.
 *
 * Output is the CSV described in sched/shape_profile.h, on stdout by default.
 *
 *   ./bench/backend_bench                  # sweep, CSV to stdout
 *   ./bench/backend_bench out.csv          # CSV to a file
 *   COLI_BENCH_ITERS=10 ./bench/backend_bench
 *   COLI_BENCH_MAX_MIB=64 ./bench/backend_bench   # cap per-shape footprint
 *
 * An engine that is not present is reported as a `# skip` comment line with the
 * backend's own reason. It is never reported as a slow measurement, and no
 * engine ever computes another engine's row: a missing engine simply has no
 * records, and placement then has nothing to rank it with.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cpu/vnni_cpu_backend.h"
#include "gpu/vulkan_backend.h"
#include "npu/xdna2_backend.h"
#include "sched/npu_shapes.h"
#include "sched/shape_profile.h"

/* Row counts, covering decode, small prefill and bulk prefill. */
static const int k_rows[] = { 1, 8, 32, 64, 256, 512 };
#define K_ROW_COUNT (sizeof(k_rows) / sizeof(k_rows[0]))

#define DEFAULT_ITERS   5
#define DEFAULT_MAX_MIB 256

static uint64_t now_ns(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static int env_int(const char *name, int fallback) {
    const char *value = getenv(name);
    if (!value || !*value) return fallback;
    char *end = NULL;
    errno = 0;
    long parsed = strtol(value, &end, 10);
    if (errno != 0 || !end || end == value || *end != '\0' || parsed <= 0) {
        fprintf(stderr, "bench: ignoring %s='%s'\n", name, value);
        return fallback;
    }
    return (int)parsed;
}

typedef struct {
    int8_t *input;
    int8_t *weights;
    float  *scales;
    float  *output;
} buffers_t;

static void buffers_free(buffers_t *b) {
    free(b->input);
    free(b->weights);
    free(b->scales);
    free(b->output);
    memset(b, 0, sizeof(*b));
}

static int buffers_alloc(buffers_t *b, int rows, int inner, int out) {
    memset(b, 0, sizeof(*b));
    b->input   = (int8_t *)malloc((size_t)rows * (size_t)inner);
    b->weights = (int8_t *)malloc((size_t)out * (size_t)inner);
    b->scales  = (float *)malloc((size_t)out * sizeof(float));
    b->output  = (float *)malloc((size_t)rows * (size_t)out * sizeof(float));
    if (!b->input || !b->weights || !b->scales || !b->output) {
        buffers_free(b);
        return -ENOMEM;
    }
    for (size_t i = 0; i < (size_t)rows * (size_t)inner; ++i) {
        b->input[i] = (int8_t)((i * 3 + 1) % 17 - 8);
    }
    for (size_t i = 0; i < (size_t)out * (size_t)inner; ++i) {
        b->weights[i] = (int8_t)((i * 5 + 3) % 13 - 6);
    }
    for (int i = 0; i < out; ++i) b->scales[i] = 1.0f / 128.0f;
    memset(b->output, 0, (size_t)rows * (size_t)out * sizeof(float));
    return 0;
}

static void record_init(coli_shape_record_t *record, coli_engine_t engine,
                        int rows, int inner, int out, int iters) {
    memset(record, 0, sizeof(*record));
    record->engine = engine;
    record->rows = rows;
    record->inner = inner;
    record->out = out;
    record->fmt = XDNA2_FMT_INT8;
    record->iterations = iters;
    record->fixed_ns    = COLI_PROFILE_UNMEASURED;
    record->alloc_ns    = COLI_PROFILE_UNMEASURED;
    record->upload_ns   = COLI_PROFILE_UNMEASURED;
    record->submit_ns   = COLI_PROFILE_UNMEASURED;
    record->wait_ns     = COLI_PROFILE_UNMEASURED;
    record->readback_ns = COLI_PROFILE_UNMEASURED;
    record->teardown_ns = COLI_PROFILE_UNMEASURED;
}

/* ── Per-engine measurement ── */

static int measure_cpu(const buffers_t *b, int rows, int inner, int out,
                       int iters, coli_shape_record_t *record) {
    record_init(record, COLI_ENGINE_CPU, rows, inner, out, iters);

    /* One untimed call so the first-touch page faults are not charged to the
     * measurement; every engine gets the same treatment. */
    if (!strix_cpu_matmul(b->input, rows, inner, b->weights, out,
                          b->output, b->scales)) {
        return -EIO;
    }

    const uint64_t start = now_ns();
    for (int i = 0; i < iters; ++i) {
        if (!strix_cpu_matmul(b->input, rows, inner, b->weights, out,
                              b->output, b->scales)) {
            return -EIO;
        }
    }
    const uint64_t elapsed = now_ns() - start;

    record->total_ns = (double)elapsed / (double)iters;
    /* The VNNI path computes in place out of host memory: there is no upload,
     * no submit and no readback. Zero here is a measurement, not a gap. */
    record->fixed_ns = 0.0;
    return 0;
}

static int measure_gpu(const buffers_t *b, int rows, int inner, int out,
                       int iters, coli_shape_record_t *record) {
    record_init(record, COLI_ENGINE_GPU, rows, inner, out, iters);

    if (!strix_vulkan_matmul(b->input, rows, inner, b->weights, out,
                             b->output, b->scales)) {
        return -EIO;
    }

    const uint64_t start = now_ns();
    for (int i = 0; i < iters; ++i) {
        if (!strix_vulkan_matmul(b->input, rows, inner, b->weights, out,
                                 b->output, b->scales)) {
            return -EIO;
        }
    }
    const uint64_t elapsed = now_ns() - start;

    record->total_ns = (double)elapsed / (double)iters;
    /* The Vulkan backend owns its staging internally and does not report a
     * stage breakdown. Leaving fixed_ns unmeasured is honest; inventing a split
     * would put a made-up number into the placement table. */
    return 0;
}

static int measure_npu(const buffers_t *b, int rows, int inner, int out,
                       int iters, coli_shape_record_t *record) {
    record_init(record, COLI_ENGINE_NPU, rows, inner, out, iters);

    xdna2_matmul_timing_t timing;
    if (!strix_xdna2_matmul_timed(b->input, rows, inner, b->weights, out,
                                  b->output, b->scales, &timing)) {
        /* No artifact for this exact shape. Fixed-shape hardware: that is a
         * skip, and emphatically not something to measure on another engine. */
        return -ENOENT;
    }

    double alloc = 0, upload = 0, submit = 0, wait = 0, readback = 0;
    double teardown = 0, total = 0;
    for (int i = 0; i < iters; ++i) {
        if (!strix_xdna2_matmul_timed(b->input, rows, inner, b->weights, out,
                                      b->output, b->scales, &timing)) {
            return -EIO;
        }
        alloc    += (double)timing.alloc_ns;
        upload   += (double)timing.upload_ns;
        submit   += (double)timing.submit_ns;
        wait     += (double)timing.wait_ns;
        readback += (double)timing.readback_ns;
        teardown += (double)timing.teardown_ns;
        total    += (double)timing.total_ns;
    }

    const double n = (double)iters;
    record->alloc_ns    = alloc / n;
    record->upload_ns   = upload / n;
    record->submit_ns   = submit / n;
    record->wait_ns     = wait / n;
    record->readback_ns = readback / n;
    record->teardown_ns = teardown / n;
    record->total_ns    = total / n;
    /* Only the syncobj wait contains array time. */
    record->fixed_ns    = record->total_ns - record->wait_ns;
    if (record->fixed_ns < 0.0) record->fixed_ns = 0.0;
    return 0;
}

/* ── Sweep ── */

int main(int argc, char **argv) {
    const int iters = env_int("COLI_BENCH_ITERS", DEFAULT_ITERS);
    const size_t max_bytes =
        (size_t)env_int("COLI_BENCH_MAX_MIB", DEFAULT_MAX_MIB) * 1024u * 1024u;

    const char *out_path = (argc > 1) ? argv[1] : getenv("COLI_BENCH_OUTPUT");
    FILE *out = stdout;
    if (out_path && *out_path) {
        out = fopen(out_path, "w");
        if (!out) {
            fprintf(stderr, "bench: cannot write '%s': %s\n",
                    out_path, strerror(errno));
            return 1;
        }
    }

    const int cpu_ok = strix_cpu_is_supported();
    const int gpu_ok = strix_vulkan_is_supported();
    const int npu_ok = strix_xdna2_is_supported();

    fprintf(out, "# colibri Strix Halo int8 matmul profile\n");
    fprintf(out, "# iterations per shape: %d\n", iters);
    if (!cpu_ok) fprintf(out, "# skip cpu: %s\n", strix_cpu_backend_name());
    if (!gpu_ok) fprintf(out, "# skip gpu: %s\n", strix_vulkan_failure_reason());
    if (!npu_ok) fprintf(out, "# skip npu: %s\n", strix_xdna2_failure_reason());
    coli_profile_write_header(out);
    fflush(out);

    size_t projection_count = 0;
    const coli_npu_projection_t *projections = coli_npu_projections(&projection_count);

    int measured = 0;
    for (size_t p = 0; p < projection_count; ++p) {
        const int inner = projections[p].inner;
        const int out_cols = projections[p].out;

        for (size_t r = 0; r < K_ROW_COUNT; ++r) {
            const int rows = k_rows[r];
            const size_t footprint = coli_npu_operand_bytes(rows, inner, out_cols);
            if (footprint == 0 || footprint > max_bytes) {
                fprintf(out, "# skip %dx%dx%d: %zu bytes exceeds COLI_BENCH_MAX_MIB\n",
                        rows, inner, out_cols, footprint);
                continue;
            }

            buffers_t buffers;
            if (buffers_alloc(&buffers, rows, inner, out_cols) != 0) {
                fprintf(out, "# skip %dx%dx%d: out of host memory\n",
                        rows, inner, out_cols);
                continue;
            }

            coli_shape_record_t record;
            if (cpu_ok && measure_cpu(&buffers, rows, inner, out_cols,
                                      iters, &record) == 0) {
                coli_profile_write_record(out, &record);
                measured++;
            }
            if (gpu_ok && measure_gpu(&buffers, rows, inner, out_cols,
                                      iters, &record) == 0) {
                coli_profile_write_record(out, &record);
                measured++;
            }
            if (npu_ok) {
                int ret = measure_npu(&buffers, rows, inner, out_cols,
                                      iters, &record);
                if (ret == 0) {
                    coli_profile_write_record(out, &record);
                    measured++;
                } else if (ret == -ENOENT) {
                    fprintf(out, "# skip npu %dx%dx%d: no .npukernel artifact\n",
                            rows, inner, out_cols);
                }
            }
            fflush(out);
            buffers_free(&buffers);
        }
    }

    fprintf(out, "# %d record(s)\n", measured);
    if (out != stdout) fclose(out);

    strix_vulkan_shutdown();
    strix_xdna2_shutdown();

    if (measured == 0) {
        fprintf(stderr,
                "bench: no engine produced a record on this host; the placement "
                "table cannot be built here\n");
        /* Not a failure: on a host without the silicon there is nothing to
         * measure, and the suite must still be runnable. */
    }
    return 0;
}
