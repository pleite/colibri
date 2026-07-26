/**
 * npu_shapes.c — Qwen 3.6 MoE shape set and host-side row tiling.
 *
 * See npu_shapes.h. Nothing here dispatches; this is the enumeration the NPU
 * backend and the placement policy agree on.
 */

#include "npu_shapes.h"

#include <errno.h>
#include <stdio.h>

/*
 * The model's distinct (inner, out) projections. Derived from the Qwen3.5/3.6
 * MoE geometry recorded in COPILOT_PROMPT_QWEN35MOE.md: hidden 4096, expert FFN
 * 1024, 512 routed experts, q_proj [16384,4096], k/v_proj [512,4096],
 * o_proj [4096,8192], router gate [512,4096].
 *
 * Weight tensors are stored [out][inner], so a [1024,4096] gate_proj is the
 * (inner=4096, out=1024) pair here.
 */
static const coli_npu_projection_t k_projections[] = {
    { 4096,  1024,  "expert gate_proj/up_proj, shared expert gate/up" },
    { 1024,  4096,  "expert down_proj, shared expert down" },
    { 4096, 16384,  "self_attn q_proj" },
    { 4096,   512,  "self_attn k_proj/v_proj, MoE router gate" },
    { 8192,  4096,  "self_attn o_proj" },
};

static const int k_row_tiles[] = {
    COLI_NPU_ROW_TILE_PREFILL_LARGE,
    COLI_NPU_ROW_TILE_PREFILL_SMALL,
    COLI_NPU_ROW_TILE_DECODE,
};

#define K_PROJECTION_COUNT (sizeof(k_projections) / sizeof(k_projections[0]))
#define K_ROW_TILE_COUNT   (sizeof(k_row_tiles) / sizeof(k_row_tiles[0]))

/* The cross product, materialised once so callers can iterate it directly. */
static coli_npu_shape_t g_shape_set[K_PROJECTION_COUNT * K_ROW_TILE_COUNT];
static int g_shape_set_built = 0;

static void build_shape_set(void) {
    if (g_shape_set_built) return;
    size_t n = 0;
    for (size_t t = 0; t < K_ROW_TILE_COUNT; ++t) {
        for (size_t p = 0; p < K_PROJECTION_COUNT; ++p) {
            g_shape_set[n].rows  = k_row_tiles[t];
            g_shape_set[n].inner = k_projections[p].inner;
            g_shape_set[n].out   = k_projections[p].out;
            g_shape_set[n].role  = k_projections[p].role;
            n++;
        }
    }
    g_shape_set_built = 1;
}

const coli_npu_projection_t *coli_npu_projections(size_t *count) {
    if (count) *count = K_PROJECTION_COUNT;
    return k_projections;
}

const int *coli_npu_row_tiles(size_t *count) {
    if (count) *count = K_ROW_TILE_COUNT;
    return k_row_tiles;
}

const coli_npu_shape_t *coli_npu_shape_set(size_t *count) {
    build_shape_set();
    if (count) *count = K_PROJECTION_COUNT * K_ROW_TILE_COUNT;
    return g_shape_set;
}

bool coli_npu_projection_is_known(int inner, int out) {
    for (size_t p = 0; p < K_PROJECTION_COUNT; ++p) {
        if (k_projections[p].inner == inner && k_projections[p].out == out) {
            return true;
        }
    }
    return false;
}

bool coli_npu_shape_is_member(int rows, int inner, int out) {
    if (!coli_npu_projection_is_known(inner, out)) return false;
    for (size_t t = 0; t < K_ROW_TILE_COUNT; ++t) {
        if (k_row_tiles[t] == rows) return true;
    }
    return false;
}

int coli_npu_shape_filename(int rows, int inner, int out,
                            char *buffer, size_t buffer_bytes) {
    if (!buffer || buffer_bytes == 0 || rows <= 0 || inner <= 0 || out <= 0) {
        return -EINVAL;
    }
    int n = snprintf(buffer, buffer_bytes, "matmul_int8_%dx%dx%d.npukernel",
                     rows, inner, out);
    if (n < 0) return -EINVAL;
    if ((size_t)n >= buffer_bytes) return -ENOSPC;
    return n;
}

/* ── Row tiling ── */

int coli_npu_plan_tiles(int rows, int inner, int out, coli_npu_tiling_t *out_plan) {
    if (!out_plan || rows <= 0) return -EINVAL;

    out_plan->count = 0;
    out_plan->inner = inner;
    out_plan->out = out;
    out_plan->padded_rows = 0;
    out_plan->complete = false;

    if (!coli_npu_projection_is_known(inner, out)) {
        /* Deliberate: no nearest-projection match. A kernel compiled for a
         * different inner/out would read a buffer it was not built for. */
        return -ENOENT;
    }

    int offset = 0;
    int remaining = rows;
    for (size_t t = 0; t < K_ROW_TILE_COUNT && remaining > 0; ++t) {
        const int tile = k_row_tiles[t];
        while (remaining >= tile) {
            if (out_plan->count >= COLI_NPU_MAX_TILES) {
                out_plan->count = 0;
                out_plan->padded_rows = 0;
                return -E2BIG;
            }
            out_plan->tiles[out_plan->count].row_offset = offset;
            out_plan->tiles[out_plan->count].rows = tile;
            out_plan->count++;
            offset += tile;
            remaining -= tile;
        }
    }

    /* The tile set contains 1, so a greedy pass always terminates exactly. */
    out_plan->padded_rows = offset;
    out_plan->complete = (remaining == 0);
    return out_plan->complete ? 0 : -E2BIG;
}

size_t coli_npu_operand_bytes(int rows, int inner, int out) {
    if (rows <= 0 || inner <= 0 || out <= 0) return 0;
    const size_t r = (size_t)rows, i = (size_t)inner, o = (size_t)out;
    return r * i              /* int8 activations */
         + o * i              /* int8 weights */
         + r * o * sizeof(float); /* f32 output */
}
