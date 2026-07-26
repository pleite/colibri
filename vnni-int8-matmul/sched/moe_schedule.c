/**
 * moe_schedule.c — router output to engine work for Qwen 3.6 MoE.
 *
 * See moe_schedule.h. Nothing here dispatches or allocates device memory; this
 * is the planning half, so it runs and is tested on a machine with none of the
 * silicon.
 */

#include "moe_schedule.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* ── Grouping ── */

void coli_moe_grouping_init(coli_moe_grouping_t *grouping) {
    if (!grouping) return;
    memset(grouping, 0, sizeof(*grouping));
}

void coli_moe_grouping_free(coli_moe_grouping_t *grouping) {
    if (!grouping) return;
    free(grouping->order);
    free(grouping->groups);
    memset(grouping, 0, sizeof(*grouping));
}

/*
 * Counting sort on the expert id. A comparison sort would work too, but the
 * expert id is a small dense integer and the router calls this once per layer
 * per step: an O(n + experts) pass keeps the scheduler off the critical path,
 * and it is stable by construction, which the combine step depends on.
 */
int coli_moe_group_by_expert(const coli_moe_assignment_t *assignments,
                             int count,
                             coli_moe_grouping_t *grouping) {
    if (!grouping) return -EINVAL;
    coli_moe_grouping_free(grouping);
    if (count < 0 || (count > 0 && !assignments)) return -EINVAL;
    if (count == 0) return 0;

    int max_expert = -1;
    for (int i = 0; i < count; ++i) {
        if (assignments[i].expert < 0) return -EINVAL;
        if (assignments[i].expert > max_expert) max_expert = assignments[i].expert;
    }

    const size_t bucket_count = (size_t)max_expert + 1;
    int *counts = (int *)calloc(bucket_count, sizeof(*counts));
    if (!counts) return -ENOMEM;

    for (int i = 0; i < count; ++i) counts[assignments[i].expert]++;

    int distinct = 0;
    for (size_t e = 0; e < bucket_count; ++e) {
        if (counts[e] > 0) distinct++;
    }

    int *order = (int *)malloc((size_t)count * sizeof(*order));
    coli_moe_group_t *groups =
        (coli_moe_group_t *)malloc((size_t)distinct * sizeof(*groups));
    int *cursor = (int *)calloc(bucket_count, sizeof(*cursor));
    if (!order || !groups || !cursor) {
        free(order);
        free(groups);
        free(cursor);
        free(counts);
        return -ENOMEM;
    }

    /* Group starts, in ascending expert order. */
    int running = 0;
    int group_index = 0;
    for (size_t e = 0; e < bucket_count; ++e) {
        if (counts[e] == 0) continue;
        groups[group_index].expert = (int)e;
        groups[group_index].first = running;
        groups[group_index].rows = counts[e];
        cursor[e] = running;
        running += counts[e];
        group_index++;
    }

    /* Stable scatter: assignments keep their relative order within an expert. */
    for (int i = 0; i < count; ++i) {
        order[cursor[assignments[i].expert]++] = i;
    }

    free(cursor);
    free(counts);

    grouping->order = order;
    grouping->groups = groups;
    grouping->group_count = distinct;
    grouping->row_count = count;
    grouping->capacity_rows = count;
    grouping->capacity_groups = distinct;
    return 0;
}

/* ── Lanes ── */

static int env_lane_count(const char *name) {
    const char *value = getenv(name);
    if (!value || !*value) return 0;
    char *end = NULL;
    errno = 0;
    long parsed = strtol(value, &end, 10);
    if (errno != 0 || !end || end == value || *end != '\0') return 0;
    if (parsed < 1 || parsed > 1024) return 0;
    return (int)parsed;
}

void coli_moe_lane_limits_from_env(coli_moe_lane_limits_t *limits) {
    if (!limits) return;
    const int cpu = env_lane_count("COLI_MOE_CPU_LANES");
    const int gpu = env_lane_count("COLI_MOE_GPU_LANES");
    limits->cpu_lanes = cpu > 0 ? cpu : 1;
    limits->gpu_lanes = gpu > 0 ? gpu : 1;
    /*
     * Not configurable. The XDNA 2 runtime owns one hardware context on one AIE
     * partition; a second context over the same columns fails at CREATE_HWCTX,
     * so "more NPU lanes" would mean concurrent submissions through one
     * context, which is a different feature.
     */
    limits->npu_lanes = COLI_MOE_NPU_LANES;
}

int coli_moe_lane_limit(const coli_moe_lane_limits_t *limits, coli_engine_t engine) {
    if (!limits) return 0;
    switch (engine) {
        case COLI_ENGINE_CPU: return limits->cpu_lanes > 0 ? limits->cpu_lanes : 1;
        case COLI_ENGINE_GPU: return limits->gpu_lanes > 0 ? limits->gpu_lanes : 1;
        case COLI_ENGINE_NPU: return COLI_MOE_NPU_LANES;
        default:              return 0;
    }
}

/* ── Plan ── */

void coli_moe_plan_init(coli_moe_plan_t *plan) {
    if (!plan) return;
    memset(plan, 0, sizeof(*plan));
}

void coli_moe_plan_free(coli_moe_plan_t *plan) {
    if (!plan) return;
    free(plan->items);
    memset(plan, 0, sizeof(*plan));
}

int coli_moe_plan_build(const coli_moe_grouping_t *grouping,
                        const coli_placement_policy_t *policy,
                        const coli_placement_caps_t *caps,
                        const coli_moe_lane_limits_t *limits,
                        int inner, int out, int fmt,
                        coli_moe_plan_t *plan) {
    return coli_moe_plan_build_resident(grouping, policy, caps, limits,
                                        inner, out, fmt, NULL, NULL, plan);
}

int coli_moe_plan_build_resident(const coli_moe_grouping_t *grouping,
                                 const coli_placement_policy_t *policy,
                                 const coli_placement_caps_t *caps,
                                 const coli_moe_lane_limits_t *limits,
                                 int inner, int out, int fmt,
                                 coli_moe_weights_resident_fn resident,
                                 void *resident_user,
                                 coli_moe_plan_t *plan) {
    if (!grouping || !policy || !caps || !limits || !plan) return -EINVAL;
    if (inner <= 0 || out <= 0) return -EINVAL;

    coli_moe_plan_free(plan);
    if (grouping->group_count <= 0) return 0;

    coli_moe_plan_item_t *items = (coli_moe_plan_item_t *)calloc(
        (size_t)grouping->group_count, sizeof(*items));
    if (!items) return -ENOMEM;

    int next_lane[COLI_ENGINE_COUNT_];
    memset(next_lane, 0, sizeof(next_lane));

    int unplaced = 0;
    for (int g = 0; g < grouping->group_count; ++g) {
        const coli_moe_group_t *group = &grouping->groups[g];

        /* Per-group residency: the same expert can be warm on one engine and
         * cold on another, and that is the whole point of asking. */
        coli_placement_caps_t group_caps = *caps;
        if (resident) {
            for (int e = COLI_ENGINE_CPU; e < COLI_ENGINE_COUNT_; ++e) {
                group_caps.weights_resident[e] =
                    resident(group->expert, (coli_engine_t)e, resident_user);
            }
        }

        coli_placement_decision_t decision;
        coli_engine_t engine = coli_choose_backend(policy, &group_caps, group->rows,
                                                   inner, out, fmt, &decision);

        items[g].group_index = g;
        items[g].expert = group->expert;
        items[g].rows = group->rows;
        items[g].engine = engine;
        items[g].reason = decision.reason;
        items[g].source = decision.source;

        if (engine == COLI_ENGINE_NONE) {
            items[g].lane = -1;
            unplaced++;
            continue;
        }

        const int lanes = coli_moe_lane_limit(limits, engine);
        items[g].lane = (lanes > 0) ? (next_lane[engine] % lanes) : 0;
        if (lanes > 0) next_lane[engine] = (next_lane[engine] + 1) % lanes;
    }

    plan->items = items;
    plan->count = grouping->group_count;
    plan->capacity = grouping->group_count;
    plan->unplaced = unplaced;
    return 0;
}

int coli_moe_plan_engine_items(const coli_moe_plan_t *plan, coli_engine_t engine) {
    if (!plan) return 0;
    int n = 0;
    for (int i = 0; i < plan->count; ++i) {
        if (plan->items[i].engine == engine) n++;
    }
    return n;
}

/* ── Execution ── */

int coli_moe_plan_execute(const coli_moe_plan_t *plan,
                          const coli_moe_lane_limits_t *limits,
                          coli_moe_dispatch_fn dispatch,
                          void *user,
                          coli_moe_exec_stats_t *stats) {
    coli_moe_exec_stats_t local;
    if (!stats) stats = &local;
    memset(stats, 0, sizeof(*stats));
    stats->failed_item = -1;

    if (!plan || !limits || !dispatch) return -EINVAL;
    if (plan->count > 0 && !plan->items) return -EINVAL;
    if (plan->unplaced > 0) {
        /* Running the placeable part would drop the rest of the tokens on the
         * floor. An unplaceable plan is a scheduling failure, not a partial
         * success. */
        return -EINVAL;
    }
    if (plan->count <= 0) return 0;

    bool *done = (bool *)calloc((size_t)plan->count, sizeof(*done));
    if (!done) return -ENOMEM;

    int remaining = plan->count;
    while (remaining > 0) {
        int issued_this_wave[COLI_ENGINE_COUNT_];
        memset(issued_this_wave, 0, sizeof(issued_this_wave));
        int issued = 0;

        for (int i = 0; i < plan->count; ++i) {
            if (done[i]) continue;
            const coli_moe_plan_item_t *item = &plan->items[i];
            const coli_engine_t engine = item->engine;
            if (engine <= COLI_ENGINE_NONE || engine >= COLI_ENGINE_COUNT_) {
                free(done);
                return -EINVAL;
            }
            const int lanes = coli_moe_lane_limit(limits, engine);
            if (lanes <= 0) {
                free(done);
                return -EINVAL;
            }
            if (issued_this_wave[engine] >= lanes) continue;

            const int rc = dispatch(item, user);
            if (rc != 0) {
                stats->failed_item = i;
                stats->failed_rc = rc;
                free(done);
                return rc;
            }

            done[i] = true;
            remaining--;
            issued++;
            issued_this_wave[engine]++;
            stats->dispatched++;
            stats->per_engine[engine]++;
            if (issued_this_wave[engine] > stats->max_in_flight[engine]) {
                stats->max_in_flight[engine] = issued_this_wave[engine];
            }
        }

        stats->waves++;
        if (issued == 0) {
            /* No engine could take anything: the lane limits contradict the
             * plan. Spinning here would hang the caller. */
            free(done);
            return -EINVAL;
        }
    }

    free(done);
    return 0;
}

/* ── Expert weight residency ── */

typedef struct {
    int    expert;
    size_t bytes;
    uint64_t last_used;  /* monotonically increasing use counter */
    bool   occupied;
} residency_entry_t;

struct coli_moe_residency {
    residency_entry_t *entries;
    int    capacity;
    int    count;
    size_t budget_bytes;
    size_t used_bytes;
    uint64_t clock;
};

coli_moe_residency_t *coli_moe_residency_create(size_t budget_bytes,
                                                int max_experts) {
    if (budget_bytes == 0 || max_experts <= 0) return NULL;
    coli_moe_residency_t *residency =
        (coli_moe_residency_t *)calloc(1, sizeof(*residency));
    if (!residency) return NULL;
    residency->entries =
        (residency_entry_t *)calloc((size_t)max_experts, sizeof(residency_entry_t));
    if (!residency->entries) {
        free(residency);
        return NULL;
    }
    residency->capacity = max_experts;
    residency->budget_bytes = budget_bytes;
    return residency;
}

void coli_moe_residency_free(coli_moe_residency_t *residency) {
    if (!residency) return;
    free(residency->entries);
    free(residency);
}

static residency_entry_t *find_entry(coli_moe_residency_t *residency, int expert) {
    for (int i = 0; i < residency->capacity; ++i) {
        if (residency->entries[i].occupied && residency->entries[i].expert == expert) {
            return &residency->entries[i];
        }
    }
    return NULL;
}

static residency_entry_t *find_free(coli_moe_residency_t *residency) {
    for (int i = 0; i < residency->capacity; ++i) {
        if (!residency->entries[i].occupied) return &residency->entries[i];
    }
    return NULL;
}

static residency_entry_t *find_lru(coli_moe_residency_t *residency) {
    residency_entry_t *lru = NULL;
    for (int i = 0; i < residency->capacity; ++i) {
        residency_entry_t *e = &residency->entries[i];
        if (!e->occupied) continue;
        if (!lru || e->last_used < lru->last_used) lru = e;
    }
    return lru;
}

int coli_moe_residency_touch(coli_moe_residency_t *residency,
                             int expert, size_t bytes,
                             bool *was_resident,
                             int *evicted, int evicted_capacity,
                             int *evicted_count) {
    if (!residency || expert < 0 || bytes == 0) return -EINVAL;
    if (evicted_capacity < 0 || (evicted_capacity > 0 && !evicted)) return -EINVAL;
    if (was_resident) *was_resident = false;
    if (evicted_count) *evicted_count = 0;

    if (bytes > residency->budget_bytes) {
        /* One expert does not fit at all. Evicting the entire cache would not
         * help, and pretending it is resident would corrupt the next dispatch. */
        return -ENOSPC;
    }

    residency->clock++;

    residency_entry_t *existing = find_entry(residency, expert);
    if (existing) {
        existing->last_used = residency->clock;
        if (existing->bytes == bytes) {
            if (was_resident) *was_resident = true;
            return 0;
        }
        /* Size changed (a different projection): treat it as a re-upload. */
        residency->used_bytes -= existing->bytes;
        existing->bytes = bytes;
        residency->used_bytes += bytes;
        return 0;
    }

    int evictions = 0;
    while (residency->used_bytes + bytes > residency->budget_bytes ||
           residency->count >= residency->capacity) {
        residency_entry_t *victim = find_lru(residency);
        if (!victim) break;  /* nothing resident; the budget check above holds */
        if (evictions < evicted_capacity) evicted[evictions] = victim->expert;
        evictions++;
        residency->used_bytes -= victim->bytes;
        residency->count--;
        victim->occupied = false;
        victim->bytes = 0;
    }
    if (evicted_count) *evicted_count = evictions;

    residency_entry_t *slot = find_free(residency);
    if (!slot) return -ENOSPC;
    slot->occupied = true;
    slot->expert = expert;
    slot->bytes = bytes;
    slot->last_used = residency->clock;
    residency->count++;
    residency->used_bytes += bytes;
    return 0;
}

bool coli_moe_residency_contains(const coli_moe_residency_t *residency, int expert) {
    if (!residency) return false;
    for (int i = 0; i < residency->capacity; ++i) {
        if (residency->entries[i].occupied && residency->entries[i].expert == expert) {
            return true;
        }
    }
    return false;
}

size_t coli_moe_residency_bytes(const coli_moe_residency_t *residency) {
    return residency ? residency->used_bytes : 0;
}

int coli_moe_residency_count(const coli_moe_residency_t *residency) {
    return residency ? residency->count : 0;
}
