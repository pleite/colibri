#ifndef COLI_SHAPE_PROFILE_H
#define COLI_SHAPE_PROFILE_H

/**
 * shape_profile.h — measured cost of one int8 matmul shape on one Strix Halo
 * engine.
 *
 * Placement decisions are made from *measurements*, not from constants baked
 * into C. `bench/backend_bench.c` produces the table on the machine that has
 * the silicon and writes it as CSV; this module is the only thing that knows
 * the format, and it is what `backend_placement.c` consults.
 *
 * A record separates fixed cost from array time on purpose. At S=1 the NPU
 * spends almost all of a dispatch in BO allocation, cache maintenance, submit
 * and readback; a single "this shape took N ns" number hides that and would
 * make the decode-path decision meaningless.
 *
 * CSV format (one header line, then one record per line):
 *
 *   backend,rows,inner,out,fmt,iters,total_ns,fixed_ns,alloc_ns,upload_ns,
 *   submit_ns,wait_ns,readback_ns,teardown_ns
 *
 * `backend` is cpu|gpu|npu. Times are per iteration, in nanoseconds. A stage a
 * backend cannot attribute is written as -1, never as 0: zero means "measured
 * and negligible", -1 means "not measured".
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Engines ── */

typedef enum {
    COLI_ENGINE_NONE = 0,
    COLI_ENGINE_CPU  = 1,  /* AVX-512 VNNI */
    COLI_ENGINE_GPU  = 2,  /* RADV GFX1151 iGPU, headless Vulkan compute */
    COLI_ENGINE_NPU  = 3,  /* XDNA 2 */
    COLI_ENGINE_COUNT_
} coli_engine_t;

/** "cpu", "gpu", "npu"; "none" for COLI_ENGINE_NONE. */
const char *coli_engine_name(coli_engine_t engine);

/** Parse "cpu"/"gpu"/"npu" (case-insensitive). Returns COLI_ENGINE_NONE. */
coli_engine_t coli_engine_parse(const char *name);

/* ── Records ── */

/** A stage that was not measured is recorded as this value. */
#define COLI_PROFILE_UNMEASURED (-1.0)

typedef struct {
    coli_engine_t engine;
    int  rows;
    int  inner;
    int  out;
    int  fmt;          /* XDNA2_FMT_INT8 == 1 */
    int  iterations;
    double total_ns;    /* per iteration, wall clock */
    double fixed_ns;    /* per iteration, everything that is not array time */
    double alloc_ns;
    double upload_ns;
    double submit_ns;
    double wait_ns;
    double readback_ns;
    double teardown_ns;
} coli_shape_record_t;

typedef struct coli_shape_profile coli_shape_profile_t;

/* ── Table lifecycle ── */

/** Allocate an empty table. Returns NULL on allocation failure. */
coli_shape_profile_t *coli_profile_create(void);

/** Release a table. Safe on NULL. */
void coli_profile_free(coli_shape_profile_t *profile);

/** Number of records held. */
size_t coli_profile_count(const coli_shape_profile_t *profile);

/** Record at `index`, or NULL when out of range. */
const coli_shape_record_t *coli_profile_at(const coli_shape_profile_t *profile,
                                           size_t index);

/**
 * Append a record. Returns 0, or -EINVAL for a malformed record (non-positive
 * shape, unknown engine, non-positive iteration count) and -ENOMEM on
 * allocation failure. A record is never silently dropped.
 */
int coli_profile_append(coli_shape_profile_t *profile,
                        const coli_shape_record_t *record);

/* ── CSV ── */

/** Write the header line. Returns 0 or -EIO. */
int coli_profile_write_header(FILE *out);

/** Write one record. Returns 0 or -EIO. */
int coli_profile_write_record(FILE *out, const coli_shape_record_t *record);

/** Write header plus every record. Returns 0 or -EIO. */
int coli_profile_write(const coli_shape_profile_t *profile, FILE *out);

/**
 * Parse a table. Blank lines and lines starting with '#' are ignored, as is a
 * leading header line. Returns 0 on success, -EINVAL on the first malformed
 * line (its number is reported through `error_line` when non-NULL), -ENOMEM,
 * or -EIO. Parsing is strict: a table that does not parse is not a table, and
 * placement must not run on a half-read one.
 */
int coli_profile_parse(coli_shape_profile_t *profile, FILE *in, int *error_line);

/** Convenience: open `path` and parse it. -ENOENT when it cannot be opened. */
int coli_profile_load_file(coli_shape_profile_t *profile, const char *path);

/* ── Lookup ── */

typedef enum {
    COLI_PROFILE_MISS     = 0, /* nothing measured for this engine */
    COLI_PROFILE_EXACT    = 1, /* a record for this exact shape */
    COLI_PROFILE_ESTIMATE = 2, /* scaled from the nearest measured shape */
} coli_profile_match_t;

/**
 * Estimated per-call cost of `(rows, inner, out, fmt)` on `engine`.
 *
 * An exact record is used as-is. Otherwise the nearest measured record for the
 * same engine and format is scaled: fixed cost is taken unchanged (it does not
 * depend on the shape in any way this harness can see) and array time is scaled
 * by the MAC ratio rows*inner*out. "Nearest" is the record with the smallest
 * log-space distance in that MAC count, which keeps a 1x4096x1024 decode shape
 * from being estimated off a 256x4096x16384 prefill record when a closer one
 * exists.
 *
 * Returns the match kind and, when it is not MISS, stores the estimate in
 * `*ns_out`. An estimate is deliberately distinguishable from a measurement so
 * that a caller can refuse to act on it.
 */
coli_profile_match_t coli_profile_estimate_ns(const coli_shape_profile_t *profile,
                                              coli_engine_t engine,
                                              int rows, int inner, int out, int fmt,
                                              double *ns_out);

#ifdef __cplusplus
}
#endif

#endif /* COLI_SHAPE_PROFILE_H */
