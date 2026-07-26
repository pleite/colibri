#ifndef COLI_MOE_SCHEDULE_H
#define COLI_MOE_SCHEDULE_H

/**
 * moe_schedule.h — turn a router's top-k output into engine work.
 *
 * The problem this solves
 * ----------------------
 * Qwen 3.6 MoE routes each token to 10 of 512 experts. Dispatching per token
 * gives every engine a rows=1 matmul, which is the one shape where the NPU's
 * fixed cost (BO allocation, cache maintenance, submit, syncobj wait, readback)
 * cannot be amortised by anything. Grouping the tokens by expert first turns
 * the same work into a handful of contiguous rows=N matmuls, which is what
 * makes an accelerator worth using at all.
 *
 * So the pipeline is: router output -> per-expert groups -> a placement
 * decision per group -> lane queues that respect each engine's concurrency.
 *
 * What is *not* here: any matmul. This module plans; the caller dispatches.
 * That separation is what lets the whole thing be tested on a host with none of
 * the silicon.
 */

#include <stdbool.h>
#include <stddef.h>

#include "backend_placement.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Router output ── */

typedef struct {
    int   token;    /* index of the token within the batch */
    int   expert;   /* routed expert id */
    float weight;   /* router probability, carried through for the combine */
} coli_moe_assignment_t;

/* ── Grouping ── */

typedef struct {
    int    expert;
    int    first;   /* index of the group's first row in the permutation */
    int    rows;    /* number of tokens routed to this expert */
} coli_moe_group_t;

typedef struct {
    /**
     * Row permutation: `order[i]` is the assignment index that supplies row i
     * of the gathered activation buffer. Groups are contiguous in this order,
     * which is what lets one matmul cover a whole expert.
     */
    int *order;
    coli_moe_group_t *groups;
    int  group_count;
    int  row_count;
    int  capacity_rows;
    int  capacity_groups;
} coli_moe_grouping_t;

/** Zero a grouping so it can be safely freed without being built. */
void coli_moe_grouping_init(coli_moe_grouping_t *grouping);

/** Release the buffers owned by a grouping. Safe on a zeroed grouping. */
void coli_moe_grouping_free(coli_moe_grouping_t *grouping);

/**
 * Group `count` assignments by expert.
 *
 * Groups come out in ascending expert order, and within a group the rows keep
 * their original token order — the combine step needs a deterministic mapping
 * back, and a stable order makes a mismatch reproducible instead of flaky.
 *
 * Returns 0, -EINVAL for a NULL argument or a negative expert id, and -ENOMEM.
 */
int coli_moe_group_by_expert(const coli_moe_assignment_t *assignments,
                             int count,
                             coli_moe_grouping_t *grouping);

/* ── Lanes ── */

/**
 * Concurrency cap per engine.
 *
 * The NPU cap is 1 and is not a tunable: the runtime creates a single AIE
 * partition (one hardware context) and a second context on the same partition
 * fails at CREATE_HWCTX. Submitting concurrently through one context is a
 * separate feature, not a lane count.
 */
#define COLI_MOE_NPU_LANES 1

typedef struct {
    int cpu_lanes;   /* >= 1 */
    int gpu_lanes;   /* >= 1 */
    int npu_lanes;   /* always COLI_MOE_NPU_LANES */
} coli_moe_lane_limits_t;

/**
 * Default limits: CPU from COLI_MOE_CPU_LANES (else 1), GPU from
 * COLI_MOE_GPU_LANES (else 1), NPU pinned to COLI_MOE_NPU_LANES. An out-of-
 * range or non-numeric override is ignored rather than silently clamped to
 * something surprising.
 */
void coli_moe_lane_limits_from_env(coli_moe_lane_limits_t *limits);

/** Lane cap for one engine, or 0 for COLI_ENGINE_NONE. */
int coli_moe_lane_limit(const coli_moe_lane_limits_t *limits, coli_engine_t engine);

/* ── Plan ── */

typedef struct {
    int           group_index;   /* index into the grouping's groups */
    int           expert;
    int           rows;
    coli_engine_t engine;
    /* Lane this item is assigned to, within [0, lane_limit(engine)). */
    int           lane;
    /* Why this engine — from the placement decision, or the refusal reason. */
    const char   *reason;
    coli_placement_source_t source;
} coli_moe_plan_item_t;

typedef struct {
    coli_moe_plan_item_t *items;
    int  count;
    int  capacity;
    /* Groups nothing could run. A plan with unplaced groups is not runnable. */
    int  unplaced;
} coli_moe_plan_t;

void coli_moe_plan_init(coli_moe_plan_t *plan);
void coli_moe_plan_free(coli_moe_plan_t *plan);

/**
 * Place every group.
 *
 * `inner` and `out` are the projection being computed for the experts (for
 * Qwen 3.6 MoE: 4096x1024 for gate/up, 1024x4096 for down). Each group gets its
 * own placement decision because the row count differs per expert — that is the
 * entire reason grouping happens before placement.
 *
 * Lanes are assigned round-robin within each engine, in group order, so the
 * plan is deterministic and a caller can replay it.
 *
 * Returns 0 on success (even when some groups are unplaced — `plan->unplaced`
 * reports that, and the caller must treat a non-zero value as a failure to
 * schedule rather than as permission to compute elsewhere), or -EINVAL/-ENOMEM.
 */
int coli_moe_plan_build(const coli_moe_grouping_t *grouping,
                        const coli_placement_policy_t *policy,
                        const coli_placement_caps_t *caps,
                        const coli_moe_lane_limits_t *limits,
                        int inner, int out, int fmt,
                        coli_moe_plan_t *plan);

/**
 * Place every group, asking `resident` whether each engine already holds the
 * expert's weights.
 *
 * This is the seam that makes a warm expert cheaper than a cold one: the
 * measured table is cold by construction (the bench re-uploads on every
 * iteration), so without this input placement charges every group for an upload
 * it may not owe. `resident` may be NULL, which reproduces the cold table
 * exactly. It is called once per (group, engine) and must not dispatch.
 *
 * Same return values as coli_moe_plan_build().
 */
typedef bool (*coli_moe_weights_resident_fn)(int expert, coli_engine_t engine,
                                             void *user);

int coli_moe_plan_build_resident(const coli_moe_grouping_t *grouping,
                                 const coli_placement_policy_t *policy,
                                 const coli_placement_caps_t *caps,
                                 const coli_moe_lane_limits_t *limits,
                                 int inner, int out, int fmt,
                                 coli_moe_weights_resident_fn resident,
                                 void *resident_user,
                                 coli_moe_plan_t *plan);

/** Number of planned items assigned to `engine`. */
int coli_moe_plan_engine_items(const coli_moe_plan_t *plan, coli_engine_t engine);

/* ── Execution ──
 *
 * The plan is data; this runs it. It still dispatches nothing itself — the
 * caller supplies the function that computes one group — but it owns the two
 * things a caller would otherwise have to reinvent per call site: the order
 * items are issued in, and the concurrency each engine is allowed.
 *
 * Ordering is by waves. Each wave issues at most `lane_limit(engine)` items per
 * engine, in group order, so an engine is never oversubscribed and the sequence
 * is deterministic and replayable. The NPU's cap is 1 (COLI_MOE_NPU_LANES), so
 * its items serialise through the single hardware context, which is what the
 * one-partition-one-hwctx constraint requires.
 *
 * There is no fallback. A group whose dispatch fails stops the run and the
 * error is returned; it is never re-placed onto another engine, because a
 * backend computing on another backend's behalf is what makes a measurement a
 * lie.
 */

/**
 * Compute one group. Returns 0 on success or a negative errno; anything
 * non-zero aborts the run.
 */
typedef int (*coli_moe_dispatch_fn)(const coli_moe_plan_item_t *item, void *user);

typedef struct {
    int waves;                            /* issue rounds performed */
    int dispatched;                       /* items dispatched successfully */
    int per_engine[COLI_ENGINE_COUNT_];   /* dispatched, per engine */
    int max_in_flight[COLI_ENGINE_COUNT_];/* largest wave width per engine */
    int failed_item;                      /* index of the failing item, else -1 */
    int failed_rc;                        /* its return value, else 0 */
} coli_moe_exec_stats_t;

/**
 * Run `plan`.
 *
 * Refuses (-EINVAL) a plan with unplaced groups: a plan that could not place
 * everything is not runnable, and running the placeable part silently would
 * drop tokens. `stats` may be NULL.
 *
 * Returns 0 when every item ran, the dispatcher's negative return value when
 * one failed, or -EINVAL for a NULL plan, limits or dispatcher.
 */
int coli_moe_plan_execute(const coli_moe_plan_t *plan,
                          const coli_moe_lane_limits_t *limits,
                          coli_moe_dispatch_fn dispatch,
                          void *user,
                          coli_moe_exec_stats_t *stats);

/* ── Expert weight residency ──
 *
 * Expert weights are re-used across tokens and across steps, while activations
 * are not. Uploading a 4096x1024 int8 expert for every group costs more than
 * the matmul it feeds, so hot experts must keep their device buffers. This is a
 * policy object: it decides what stays and what is evicted, and reports the
 * evictions so the caller can release the real buffers.
 */

typedef struct coli_moe_residency coli_moe_residency_t;

/**
 * Create an LRU residency policy holding at most `budget_bytes` of expert
 * weights, with room to track `max_experts` entries. Returns NULL on invalid
 * arguments or allocation failure.
 */
coli_moe_residency_t *coli_moe_residency_create(size_t budget_bytes,
                                                int max_experts);

void coli_moe_residency_free(coli_moe_residency_t *residency);

/**
 * Record use of `expert`, whose weights occupy `bytes`.
 *
 * On return `*was_resident` says whether the expert was already resident (no
 * upload needed). Experts evicted to make room are written to `evicted` (up to
 * `evicted_capacity` entries) and their number returned through
 * `*evicted_count`; the count can exceed the capacity, in which case the caller
 * has lost track of buffers and must treat it as an error rather than leak.
 *
 * Returns 0, -EINVAL, or -ENOSPC when a single expert exceeds the whole budget
 * (a budget that cannot hold one expert is a configuration error, not something
 * to work around by evicting everything).
 */
int coli_moe_residency_touch(coli_moe_residency_t *residency,
                             int expert, size_t bytes,
                             bool *was_resident,
                             int *evicted, int evicted_capacity,
                             int *evicted_count);

/** True when `expert` currently holds device buffers. */
bool coli_moe_residency_contains(const coli_moe_residency_t *residency, int expert);

/** Bytes currently held. */
size_t coli_moe_residency_bytes(const coli_moe_residency_t *residency);

/** Number of resident experts. */
int coli_moe_residency_count(const coli_moe_residency_t *residency);

#ifdef __cplusplus
}
#endif

#endif /* COLI_MOE_SCHEDULE_H */
