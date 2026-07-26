/**
 * shape_profile.c — CSV table of measured int8 matmul cost per engine.
 *
 * See shape_profile.h. This file contains no hardware constants: everything it
 * reports comes from a table produced by bench/backend_bench.c on the machine.
 */

#include "shape_profile.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ── Engines ── */

const char *coli_engine_name(coli_engine_t engine) {
    switch (engine) {
        case COLI_ENGINE_CPU: return "cpu";
        case COLI_ENGINE_GPU: return "gpu";
        case COLI_ENGINE_NPU: return "npu";
        default:              return "none";
    }
}

coli_engine_t coli_engine_parse(const char *name) {
    if (!name) return COLI_ENGINE_NONE;
    while (*name == ' ' || *name == '\t') name++;
    char lowered[8];
    size_t len = 0;
    while (name[len] && name[len] != ' ' && name[len] != '\t') {
        if (len + 1 >= sizeof(lowered)) return COLI_ENGINE_NONE;
        int c = (int)(unsigned char)name[len];
        lowered[len] = (char)((c >= 'A' && c <= 'Z') ? c + 32 : c);
        len++;
    }
    lowered[len] = '\0';
    if (strcmp(lowered, "cpu") == 0) return COLI_ENGINE_CPU;
    if (strcmp(lowered, "gpu") == 0) return COLI_ENGINE_GPU;
    if (strcmp(lowered, "npu") == 0) return COLI_ENGINE_NPU;
    return COLI_ENGINE_NONE;
}

/* ── Table ── */

struct coli_shape_profile {
    coli_shape_record_t *records;
    size_t count;
    size_t capacity;
};

coli_shape_profile_t *coli_profile_create(void) {
    coli_shape_profile_t *profile =
        (coli_shape_profile_t *)calloc(1, sizeof(*profile));
    return profile;
}

void coli_profile_free(coli_shape_profile_t *profile) {
    if (!profile) return;
    free(profile->records);
    free(profile);
}

size_t coli_profile_count(const coli_shape_profile_t *profile) {
    return profile ? profile->count : 0;
}

const coli_shape_record_t *coli_profile_at(const coli_shape_profile_t *profile,
                                           size_t index) {
    if (!profile || index >= profile->count) return NULL;
    return &profile->records[index];
}

int coli_profile_append(coli_shape_profile_t *profile,
                        const coli_shape_record_t *record) {
    if (!profile || !record) return -EINVAL;
    if (record->engine <= COLI_ENGINE_NONE || record->engine >= COLI_ENGINE_COUNT_) {
        return -EINVAL;
    }
    if (record->rows <= 0 || record->inner <= 0 || record->out <= 0) return -EINVAL;
    if (record->iterations <= 0) return -EINVAL;
    if (record->total_ns < 0.0) return -EINVAL;

    if (profile->count == profile->capacity) {
        size_t next = profile->capacity ? profile->capacity * 2 : 32;
        coli_shape_record_t *grown =
            (coli_shape_record_t *)realloc(profile->records, next * sizeof(*grown));
        if (!grown) return -ENOMEM;
        profile->records = grown;
        profile->capacity = next;
    }
    profile->records[profile->count++] = *record;
    return 0;
}

/* ── CSV output ── */

#define COLI_PROFILE_HEADER \
    "backend,rows,inner,out,fmt,iters,total_ns,fixed_ns,alloc_ns,upload_ns," \
    "submit_ns,wait_ns,readback_ns,teardown_ns"

int coli_profile_write_header(FILE *out) {
    if (!out) return -EINVAL;
    return (fprintf(out, "%s\n", COLI_PROFILE_HEADER) > 0) ? 0 : -EIO;
}

int coli_profile_write_record(FILE *out, const coli_shape_record_t *record) {
    if (!out || !record) return -EINVAL;
    int n = fprintf(out,
                    "%s,%d,%d,%d,%d,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
                    coli_engine_name(record->engine),
                    record->rows, record->inner, record->out, record->fmt,
                    record->iterations,
                    record->total_ns, record->fixed_ns,
                    record->alloc_ns, record->upload_ns, record->submit_ns,
                    record->wait_ns, record->readback_ns, record->teardown_ns);
    return (n > 0) ? 0 : -EIO;
}

int coli_profile_write(const coli_shape_profile_t *profile, FILE *out) {
    if (!profile || !out) return -EINVAL;
    int ret = coli_profile_write_header(out);
    if (ret != 0) return ret;
    for (size_t i = 0; i < profile->count; ++i) {
        ret = coli_profile_write_record(out, &profile->records[i]);
        if (ret != 0) return ret;
    }
    return 0;
}

/* ── CSV input ── */

static char *trim(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r') s++;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' ||
                       s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[--len] = '\0';
    }
    return s;
}

/* Split `line` into at most `max` NUL-terminated fields. Returns the count. */
static size_t split_csv(char *line, char **fields, size_t max) {
    size_t n = 0;
    char *cursor = line;
    while (n < max) {
        fields[n++] = cursor;
        char *comma = strchr(cursor, ',');
        if (!comma) break;
        *comma = '\0';
        cursor = comma + 1;
    }
    return n;
}

static int parse_int_field(const char *text, int *out) {
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || !end || end == text) return -EINVAL;
    while (*end == ' ' || *end == '\t') end++;
    if (*end != '\0') return -EINVAL;
    if (value < INT32_MIN || value > INT32_MAX) return -EINVAL;
    *out = (int)value;
    return 0;
}

static int parse_double_field(const char *text, double *out) {
    char *end = NULL;
    errno = 0;
    double value = strtod(text, &end);
    if (errno != 0 || !end || end == text) return -EINVAL;
    while (*end == ' ' || *end == '\t') end++;
    if (*end != '\0') return -EINVAL;
    if (!isfinite(value)) return -EINVAL;
    *out = value;
    return 0;
}

#define COLI_PROFILE_FIELDS 14

int coli_profile_parse(coli_shape_profile_t *profile, FILE *in, int *error_line) {
    if (!profile || !in) return -EINVAL;
    if (error_line) *error_line = 0;

    char line[512];
    int line_no = 0;
    while (fgets(line, sizeof(line), in)) {
        line_no++;
        char *text = trim(line);
        if (*text == '\0' || *text == '#') continue;

        /* A header line is data-shaped but starts with a non-engine token. */
        char *fields[COLI_PROFILE_FIELDS];
        char scratch[512];
        if (strlen(text) >= sizeof(scratch)) {
            if (error_line) *error_line = line_no;
            return -EINVAL;
        }
        memcpy(scratch, text, strlen(text) + 1);

        size_t n = split_csv(scratch, fields, COLI_PROFILE_FIELDS);
        if (n != COLI_PROFILE_FIELDS) {
            if (error_line) *error_line = line_no;
            return -EINVAL;
        }
        for (size_t i = 0; i < n; ++i) fields[i] = trim(fields[i]);

        coli_engine_t engine = coli_engine_parse(fields[0]);
        if (engine == COLI_ENGINE_NONE) {
            /* Header line: accepted exactly once, and only as the first
             * non-comment line, so a typo in an engine name is still an error. */
            if (profile->count == 0 && strcmp(fields[0], "backend") == 0) continue;
            if (error_line) *error_line = line_no;
            return -EINVAL;
        }

        coli_shape_record_t record;
        memset(&record, 0, sizeof(record));
        record.engine = engine;

        int *ints[] = { &record.rows, &record.inner, &record.out,
                        &record.fmt, &record.iterations };
        for (size_t i = 0; i < sizeof(ints) / sizeof(ints[0]); ++i) {
            if (parse_int_field(fields[1 + i], ints[i]) != 0) {
                if (error_line) *error_line = line_no;
                return -EINVAL;
            }
        }
        double *doubles[] = { &record.total_ns, &record.fixed_ns, &record.alloc_ns,
                              &record.upload_ns, &record.submit_ns, &record.wait_ns,
                              &record.readback_ns, &record.teardown_ns };
        for (size_t i = 0; i < sizeof(doubles) / sizeof(doubles[0]); ++i) {
            if (parse_double_field(fields[6 + i], doubles[i]) != 0) {
                if (error_line) *error_line = line_no;
                return -EINVAL;
            }
        }

        int ret = coli_profile_append(profile, &record);
        if (ret != 0) {
            if (error_line) *error_line = line_no;
            return ret;
        }
    }

    if (ferror(in)) return -EIO;
    return 0;
}

int coli_profile_load_file(coli_shape_profile_t *profile, const char *path) {
    if (!profile || !path || !*path) return -EINVAL;
    FILE *fp = fopen(path, "r");
    if (!fp) return -ENOENT;
    int error_line = 0;
    int ret = coli_profile_parse(profile, fp, &error_line);
    fclose(fp);
    return ret;
}

/* ── Lookup ── */

static double mac_count(int rows, int inner, int out) {
    return (double)rows * (double)inner * (double)out;
}

/* Bytes one int8 dispatch moves: activations, weights, and the f32 output. */
static double operand_bytes(int rows, int inner, int out) {
    return (double)rows * (double)inner +
           (double)inner * (double)out +
           (double)rows * (double)out * 4.0;
}

static double weight_bytes(int inner, int out) {
    return (double)inner * (double)out;
}

coli_row_class_t coli_row_class(int rows) {
    return (rows > 1) ? COLI_ROW_CLASS_PREFILL : COLI_ROW_CLASS_DECODE;
}

const char *coli_row_class_name(coli_row_class_t klass) {
    return (klass == COLI_ROW_CLASS_PREFILL) ? "prefill" : "decode";
}

/*
 * The record an estimate for this shape is based on: an exact match if there is
 * one, otherwise the nearest record in the same engine, format and row-tile
 * class. Both estimate_ns() and upload_ns() go through here so they can never
 * disagree about which measurement they are quoting.
 */
static coli_profile_match_t find_basis(const coli_shape_profile_t *profile,
                                       coli_engine_t engine,
                                       int rows, int inner, int out, int fmt,
                                       const coli_shape_record_t **basis) {
    if (basis) *basis = NULL;
    if (!profile || rows <= 0 || inner <= 0 || out <= 0) return COLI_PROFILE_MISS;

    const double want = mac_count(rows, inner, out);
    const coli_row_class_t want_class = coli_row_class(rows);
    const coli_shape_record_t *nearest = NULL;
    double nearest_distance = 0.0;

    for (size_t i = 0; i < profile->count; ++i) {
        const coli_shape_record_t *r = &profile->records[i];
        if (r->engine != engine || r->fmt != fmt) continue;

        if (r->rows == rows && r->inner == inner && r->out == out) {
            if (basis) *basis = r;
            return COLI_PROFILE_EXACT;
        }

        /* Hard class filter: decode and prefill records never estimate each
         * other, because that is precisely where fixed-cost dominance flips. */
        if (coli_row_class(r->rows) != want_class) continue;

        /* Log-space distance: a record two orders of magnitude away is a worse
         * basis than one 2x away, regardless of which side it is on. */
        const double have = mac_count(r->rows, r->inner, r->out);
        if (have <= 0.0) continue;
        const double distance = fabs(log(want / have));
        if (!nearest || distance < nearest_distance) {
            nearest = r;
            nearest_distance = distance;
        }
    }

    if (!nearest) return COLI_PROFILE_MISS;
    if (basis) *basis = nearest;
    return COLI_PROFILE_ESTIMATE;
}

coli_profile_match_t coli_profile_estimate_ns(const coli_shape_profile_t *profile,
                                              coli_engine_t engine,
                                              int rows, int inner, int out, int fmt,
                                              double *ns_out) {
    const coli_shape_record_t *basis = NULL;
    const coli_profile_match_t match =
        find_basis(profile, engine, rows, inner, out, fmt, &basis);
    if (match == COLI_PROFILE_MISS || !basis) return COLI_PROFILE_MISS;

    if (match == COLI_PROFILE_EXACT) {
        if (ns_out) *ns_out = basis->total_ns;
        return COLI_PROFILE_EXACT;
    }

    /*
     * Fixed cost does not scale with the shape; array time does. When the
     * record did not attribute a fixed cost (-1), treat the whole measurement
     * as array time rather than inventing a split.
     */
    const double want = mac_count(rows, inner, out);
    const double have = mac_count(basis->rows, basis->inner, basis->out);
    double fixed = (basis->fixed_ns >= 0.0) ? basis->fixed_ns : 0.0;
    if (fixed > basis->total_ns) fixed = basis->total_ns;
    const double array_time = basis->total_ns - fixed;

    if (ns_out) *ns_out = fixed + array_time * (want / have);
    return COLI_PROFILE_ESTIMATE;
}

coli_profile_match_t coli_profile_upload_ns(const coli_shape_profile_t *profile,
                                            coli_engine_t engine,
                                            int rows, int inner, int out, int fmt,
                                            double *ns_out) {
    if (ns_out) *ns_out = 0.0;
    const coli_shape_record_t *basis = NULL;
    const coli_profile_match_t match =
        find_basis(profile, engine, rows, inner, out, fmt, &basis);
    if (match == COLI_PROFILE_MISS || !basis) return COLI_PROFILE_MISS;

    /* An unattributed upload stage is not a measured saving. */
    if (basis->upload_ns < 0.0) return match;

    const double basis_bytes = operand_bytes(basis->rows, basis->inner, basis->out);
    if (basis_bytes <= 0.0) return match;

    /* upload_ns per byte moved, applied to the weights this call needs. */
    double ns = basis->upload_ns * (weight_bytes(inner, out) / basis_bytes);
    if (ns < 0.0) ns = 0.0;
    if (ns_out) *ns_out = ns;
    return match;
}
