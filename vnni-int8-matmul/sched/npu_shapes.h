#ifndef COLI_NPU_SHAPES_H
#define COLI_NPU_SHAPES_H

/**
 * npu_shapes.h — the fixed NPU shape set for Qwen 3.6 MoE, and the host-side
 * tiling that makes every dispatch land on a compiled kernel.
 *
 * Why an enumeration and not a search
 * -----------------------------------
 * AIE-2 runs fixed-shape kernels. Chasing whatever shape the model happens to
 * ask for means either a kernel explosion or an approximate match, and the NPU
 * guardrails forbid the second (see docs/strix-halo-npu.md §6). So the shape set
 * is closed: the model's own geometry is enumerated here, the tiler decomposes
 * any request into members of that set, and anything that cannot be decomposed
 * is refused rather than approximated.
 *
 * Model geometry (Qwen3.5/3.6 MoE, e.g. Ornith 397B — see
 * COPILOT_PROMPT_QWEN35MOE.md): hidden 4096, expert FFN width 1024, 512 routed
 * experts with top-10, 32 query heads / 2 KV heads, head dim 256. That collapses
 * to five distinct (inner, out) pairs:
 *
 *   (4096, 1024)   expert gate_proj / up_proj, shared expert gate/up
 *   (1024, 4096)   expert down_proj, shared expert down
 *   (4096, 16384)  self_attn q_proj
 *   (4096, 512)    self_attn k_proj / v_proj, and the MoE router gate
 *   (8192, 4096)   self_attn o_proj
 *
 * crossed with three row tiles — 1 (decode), 32 and 256 (prefill) — which is
 * 15 kernel artifacts. That count is what sizes XDNA2_MAX_KERNELS.
 */

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── The shape set ── */

/** Row tiles, largest first. Order matters: the tiler is greedy. */
#define COLI_NPU_ROW_TILE_PREFILL_LARGE 256
#define COLI_NPU_ROW_TILE_PREFILL_SMALL 32
#define COLI_NPU_ROW_TILE_DECODE        1

typedef struct {
    int inner;
    int out;
    const char *role;  /* what in the model needs this pair */
} coli_npu_projection_t;

typedef struct {
    int rows;
    int inner;
    int out;
    const char *role;
} coli_npu_shape_t;

/** The distinct (inner, out) projections of the model. */
const coli_npu_projection_t *coli_npu_projections(size_t *count);

/** The row tiles, largest first. */
const int *coli_npu_row_tiles(size_t *count);

/**
 * The full kernel set: every projection crossed with every row tile.
 * `*count` receives the number of artifacts the AIE toolchain must produce.
 */
const coli_npu_shape_t *coli_npu_shape_set(size_t *count);

/** True when (rows, inner, out) is a member of the set. */
bool coli_npu_shape_is_member(int rows, int inner, int out);

/** True when (inner, out) is one of the model's projections. */
bool coli_npu_projection_is_known(int inner, int out);

/**
 * Artifact file name for a shape, matching what the NPU backend looks up:
 * "matmul_int8_<rows>x<inner>x<out>.npukernel". Returns the number of bytes
 * written (excluding the NUL), or -EINVAL / -ENOSPC.
 */
int coli_npu_shape_filename(int rows, int inner, int out,
                            char *buffer, size_t buffer_bytes);

/* ── Host-side row tiling ── */

/**
 * Maximum number of tiles a decomposition can need: greedy over {256, 32, 1}
 * leaves at most 7 tiles of 32 and 31 of 1 after the 256s, so a bound of 64 is
 * comfortable for any row count that the caller should be batching at all.
 */
#define COLI_NPU_MAX_TILES 64

typedef struct {
    int row_offset;  /* first row of this tile within the request */
    int rows;        /* one of the row tiles */
} coli_npu_tile_t;

typedef struct {
    coli_npu_tile_t tiles[COLI_NPU_MAX_TILES];
    int  count;
    int  inner;
    int  out;
    int  padded_rows;  /* sum of tile rows; == requested rows, never more */
    bool complete;     /* every requested row is covered */
} coli_npu_tiling_t;

/**
 * Decompose `rows` into row tiles for the projection (inner, out).
 *
 * Returns 0 on success. Returns -ENOENT when (inner, out) is not a known
 * projection — the caller must then not use the NPU, and must not widen the
 * match. Returns -E2BIG when the decomposition needs more than
 * COLI_NPU_MAX_TILES tiles, and -EINVAL for a non-positive row count.
 *
 * The decomposition is exact: tiles never overlap and never run past `rows`,
 * because a fixed-shape kernel reads exactly the rows it was compiled for and
 * padding would feed it uninitialised activations.
 */
int coli_npu_plan_tiles(int rows, int inner, int out, coli_npu_tiling_t *out_plan);

/**
 * Bytes an int8 dispatch of (rows, inner, out) must have resident: activations,
 * weights and the f32 output. Used against the device-heap budget to decide
 * whether the NPU can hold the operand set at all.
 */
size_t coli_npu_operand_bytes(int rows, int inner, int out);

#ifdef __cplusplus
}
#endif

#endif /* COLI_NPU_SHAPES_H */
