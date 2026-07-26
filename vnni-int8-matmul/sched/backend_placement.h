#ifndef COLI_BACKEND_PLACEMENT_H
#define COLI_BACKEND_PLACEMENT_H

/**
 * backend_placement.h — one decision function for "which engine runs this
 * matmul", driven by measurements plus hard constraints.
 *
 * There is exactly one such function so that the answer cannot drift between
 * the MoE scheduler, the attention path and a benchmark. It is deliberately
 * pure: it takes the shape and a capability snapshot, and returns a decision
 * with a reason. It does not dispatch and it never falls back silently — if the
 * caller forces an engine that cannot run the shape, the decision says so and
 * the caller must fail rather than compute somewhere else.
 *
 * Ranking uses the measured table (sched/shape_profile.h) produced by
 * bench/backend_bench.c on the Strix Halo machine. Without a table the decision
 * is still made, but it is marked COLI_PLACEMENT_SOURCE_STRUCTURAL and the
 * caller can refuse it (COLI_PLACEMENT_REQUIRE_PROFILE=1), because a structural
 * default is an argument, not a measurement.
 */

#include <stdbool.h>
#include <stddef.h>

#include "shape_profile.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Capability snapshot ── */

typedef struct {
    bool cpu_available;      /* AVX-512 VNNI usable */
    bool gpu_available;      /* Vulkan compute context on the iGPU */
    bool npu_available;      /* /dev/accel/accel0 open, hardware context alive */

    /**
     * Bytes the NPU can hold resident for one dispatch. The amdxdna device heap
     * is capped at XDNA2_DEV_HEAP_MAX_BYTES (64 MiB); a caller that knows its
     * real budget should pass it, and 0 means "unknown, do not use residency as
     * a constraint".
     *
     * `strix_xdna2_resident_bytes()` (npu/xdna2_backend.h) probes it from the
     * device; `strix_vulkan_resident_bytes()` (gpu/vulkan_backend.h) does the
     * equivalent from the Vulkan heap sizes.
     */
    size_t npu_resident_bytes;

    /**
     * Bytes the iGPU can hold resident for one dispatch, from the largest
     * device-local heap. 0 means "unknown", exactly as for the NPU.
     */
    size_t gpu_resident_bytes;

    /**
     * Per engine: are this call's *weights* already uploaded there?
     *
     * On a UMA part this is the axis that decides expert placement. Every bench
     * record is cold — it uploads its operands on every iteration — so an
     * engine that already holds the weights is being over-charged by the
     * measured number, and a warm expert on the GPU can beat a cold one on the
     * CPU. Ranking subtracts the measured weight-upload share for an engine
     * flagged here, and nothing at all when the bench did not attribute an
     * upload stage. Default (all false) reproduces the cold measurement.
     */
    bool weights_resident[COLI_ENGINE_COUNT_];

    /**
     * Callback answering "is there a compiled .npukernel for exactly this
     * shape?". Placement must never assume one exists: an exact-shape artifact
     * is a hard precondition for choosing the NPU. NULL means "no kernels".
     */
    bool (*npu_kernel_exists)(int rows, int inner, int out, int fmt, void *user);
    void *npu_kernel_user;
} coli_placement_caps_t;

/** Fill `caps` with everything disabled and no kernels. */
void coli_placement_caps_init(coli_placement_caps_t *caps);

/* ── Decision ── */

typedef enum {
    COLI_PLACEMENT_SOURCE_NONE = 0,
    COLI_PLACEMENT_SOURCE_MEASURED,   /* an exact record decided it */
    COLI_PLACEMENT_SOURCE_ESTIMATED,  /* scaled from a nearby measured shape */
    COLI_PLACEMENT_SOURCE_STRUCTURAL, /* no table: documented default order */
    COLI_PLACEMENT_SOURCE_FORCED,     /* COLI_PLACEMENT pinned the engine */
} coli_placement_source_t;

/** "none", "measured", "estimated", "structural", "forced". */
const char *coli_placement_source_name(coli_placement_source_t source);

typedef struct {
    coli_engine_t engine;             /* COLI_ENGINE_NONE when nothing can run it */
    coli_placement_source_t source;
    /* Why this engine, or — when engine is NONE — which constraint refused. */
    const char *reason;
    /* Per-engine refusal reason, NULL when the engine was a candidate. */
    const char *rejected[COLI_ENGINE_COUNT_];
    /**
     * True when the matching refusal is a property of the silicon rather than
     * of this build: no artifact, driver or capability will ever make it pass.
     *
     * The only such refusal today is rows=1 on the NPU: the AIE2P int8 MAC has
     * an 8-row granularity and the smallest tiling the array can express is 64
     * rows, so a 1-row matmul is inexpressible (npu/aie/README.md). A caller
     * that would otherwise retry once the kernels are built must not retry
     * this one, and a report that lumps it in with "not built yet" is telling
     * the operator to wait for something that is never coming.
     */
    bool rejected_permanent[COLI_ENGINE_COUNT_];
    /* Estimated per-call cost, or a negative value when not estimated. */
    double estimate_ns[COLI_ENGINE_COUNT_];
    /**
     * Weight-upload cost included in `estimate_ns` and removed again for an
     * engine flagged in `caps.weights_resident`. 0.0 when nothing was measured.
     */
    double upload_ns[COLI_ENGINE_COUNT_];
    coli_profile_match_t match[COLI_ENGINE_COUNT_];
} coli_placement_decision_t;

/* ── Policy ── */

typedef struct {
    const coli_shape_profile_t *profile;  /* may be NULL */
    /** Engine pinned by the operator, or COLI_ENGINE_NONE for automatic. */
    coli_engine_t forced;
    /** Refuse to decide from anything weaker than a measurement. */
    bool require_profile;
} coli_placement_policy_t;

/**
 * Initialise a policy from the environment:
 *
 *   COLI_PLACEMENT             cpu|gpu|npu|auto   pin an engine (default auto)
 *   COLI_PLACEMENT_REQUIRE_PROFILE  0|1           refuse structural defaults
 *
 * `profile` is stored as-is and is not owned. Returns 0, or -EINVAL when
 * COLI_PLACEMENT names something that is not an engine.
 */
int coli_placement_policy_from_env(coli_placement_policy_t *policy,
                                   const coli_shape_profile_t *profile);

/** Path of the measured table: COLI_PLACEMENT_PROFILE, else the default. */
const char *coli_placement_profile_path(void);

/**
 * Choose an engine for y[rows][out] = x[rows][inner] * w^T, int8.
 *
 * Hard constraints, applied before any ranking:
 *   * an engine that is not available is never chosen;
 *   * the NPU additionally requires an exact-shape kernel artifact and, when
 *     `npu_resident_bytes` is known, an operand set that fits it; rows=1 is a
 *     *permanent* refusal, flagged in `decision.rejected_permanent`;
 *   * the iGPU likewise refuses an operand set larger than
 *     `gpu_resident_bytes` when that is known;
 *   * a forced engine that fails a constraint yields COLI_ENGINE_NONE with the
 *     constraint named — it is never quietly replaced by another engine.
 *
 * Among the survivors the cheapest engine wins, ranked from the measured table:
 * exact records first, and only when no engine has one do estimates scaled from
 * nearby shapes compete. An engine listed in `caps.weights_resident` is charged
 * the measured cost minus the measured weight-upload share, because the record
 * that produced it uploaded on every iteration. With no table at all, the
 * structural default order documented in docs/placement-policy.md is used and
 * `source` says so.
 *
 * `decision` may be NULL. Returns the chosen engine, which is
 * COLI_ENGINE_NONE when nothing can run the shape.
 */
coli_engine_t coli_choose_backend(const coli_placement_policy_t *policy,
                                  const coli_placement_caps_t *caps,
                                  int rows, int inner, int out, int fmt,
                                  coli_placement_decision_t *decision);

#ifdef __cplusplus
}
#endif

#endif /* COLI_BACKEND_PLACEMENT_H */
