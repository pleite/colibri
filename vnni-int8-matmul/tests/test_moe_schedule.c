/**
 * test_moe_schedule.c — host-side tests for router grouping, per-group
 * placement, lane caps and expert weight residency.
 *
 * Runs anywhere: the scheduler plans, it does not dispatch.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sched/moe_schedule.h"
#include "sched/npu_shapes.h"

static int g_failures = 0;

#define CHECK(cond, ...)                                     \
    do {                                                     \
        if (!(cond)) {                                       \
            fprintf(stderr, "FAIL %s:%d: ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__);                    \
            fprintf(stderr, "\n");                           \
            g_failures++;                                    \
        }                                                    \
    } while (0)

/* ── Grouping ── */

static void test_grouping_basic(void) {
    /* Four tokens, top-2 of experts {5, 2}: the interleaving a router produces. */
    const coli_moe_assignment_t assignments[] = {
        { 0, 5, 0.6f }, { 0, 2, 0.4f },
        { 1, 2, 0.7f }, { 1, 5, 0.3f },
        { 2, 5, 0.9f }, { 2, 2, 0.1f },
        { 3, 2, 0.5f }, { 3, 5, 0.5f },
    };
    const int count = (int)(sizeof(assignments) / sizeof(assignments[0]));

    coli_moe_grouping_t grouping;
    coli_moe_grouping_init(&grouping);
    CHECK(coli_moe_group_by_expert(assignments, count, &grouping) == 0,
          "grouping failed");

    CHECK(grouping.group_count == 2, "expected 2 groups, got %d",
          grouping.group_count);
    CHECK(grouping.row_count == count, "row count is %d", grouping.row_count);

    /* Ascending expert order. */
    CHECK(grouping.groups[0].expert == 2, "first group should be expert 2");
    CHECK(grouping.groups[1].expert == 5, "second group should be expert 5");

    /* Contiguous, covering every row exactly once. */
    int covered = 0;
    for (int g = 0; g < grouping.group_count; ++g) {
        CHECK(grouping.groups[g].first == covered,
              "group %d starts at %d, expected %d", g, grouping.groups[g].first,
              covered);
        CHECK(grouping.groups[g].rows == 4, "group %d has %d rows", g,
              grouping.groups[g].rows);
        covered += grouping.groups[g].rows;
    }
    CHECK(covered == count, "groups cover %d of %d rows", covered, count);

    /* Stability: within a group the original token order is preserved, which is
     * what the combine step reads back. */
    for (int g = 0; g < grouping.group_count; ++g) {
        int previous_token = -1;
        for (int r = 0; r < grouping.groups[g].rows; ++r) {
            const int index = grouping.order[grouping.groups[g].first + r];
            CHECK(index >= 0 && index < count, "permutation index %d", index);
            CHECK(assignments[index].expert == grouping.groups[g].expert,
                  "row %d of group %d belongs to expert %d", r, g,
                  assignments[index].expert);
            CHECK(assignments[index].token > previous_token,
                  "group %d is not stable in token order", g);
            previous_token = assignments[index].token;
        }
    }

    /* Every assignment appears exactly once. */
    int seen[8] = {0};
    for (int i = 0; i < count; ++i) seen[grouping.order[i]]++;
    for (int i = 0; i < count; ++i) {
        CHECK(seen[i] == 1, "assignment %d appears %d times", i, seen[i]);
    }

    coli_moe_grouping_free(&grouping);
}

static void test_grouping_edges(void) {
    coli_moe_grouping_t grouping;
    coli_moe_grouping_init(&grouping);

    CHECK(coli_moe_group_by_expert(NULL, 0, &grouping) == 0,
          "an empty batch is not an error");
    CHECK(grouping.group_count == 0, "an empty batch has no groups");

    const coli_moe_assignment_t bad[] = { { 0, -1, 1.0f } };
    CHECK(coli_moe_group_by_expert(bad, 1, &grouping) == -EINVAL,
          "a negative expert id must be rejected");

    CHECK(coli_moe_group_by_expert(bad, -1, &grouping) == -EINVAL,
          "a negative count must be rejected");
    CHECK(coli_moe_group_by_expert(bad, 1, NULL) == -EINVAL,
          "a NULL grouping must be rejected");

    /* A sparse expert id space (512 experts, 3 used) must not produce empty
     * groups: only touched experts get a group. */
    const coli_moe_assignment_t sparse[] = {
        { 0, 511, 1.0f }, { 1, 0, 1.0f }, { 2, 511, 1.0f }, { 3, 300, 1.0f },
    };
    CHECK(coli_moe_group_by_expert(sparse, 4, &grouping) == 0, "sparse failed");
    CHECK(grouping.group_count == 3, "expected 3 groups, got %d",
          grouping.group_count);
    CHECK(grouping.groups[0].expert == 0 && grouping.groups[0].rows == 1,
          "expert 0 group is wrong");
    CHECK(grouping.groups[2].expert == 511 && grouping.groups[2].rows == 2,
          "expert 511 group is wrong");
    coli_moe_grouping_free(&grouping);
}

/* ── Lane caps ── */

static void test_lane_limits(void) {
    coli_moe_lane_limits_t limits;

    unsetenv("COLI_MOE_CPU_LANES");
    unsetenv("COLI_MOE_GPU_LANES");
    coli_moe_lane_limits_from_env(&limits);
    CHECK(limits.cpu_lanes == 1 && limits.gpu_lanes == 1,
          "defaults should be one lane each");
    CHECK(limits.npu_lanes == COLI_MOE_NPU_LANES, "npu lanes must be pinned");

    setenv("COLI_MOE_CPU_LANES", "4", 1);
    setenv("COLI_MOE_GPU_LANES", "2", 1);
    coli_moe_lane_limits_from_env(&limits);
    CHECK(limits.cpu_lanes == 4 && limits.gpu_lanes == 2, "lane overrides ignored");
    CHECK(limits.npu_lanes == 1,
          "the npu cap must stay 1: one AIE partition, one hardware context");

    /* Nonsense is ignored rather than clamped to something surprising. */
    setenv("COLI_MOE_CPU_LANES", "-3", 1);
    coli_moe_lane_limits_from_env(&limits);
    CHECK(limits.cpu_lanes == 1, "a negative lane count must fall back to 1");
    setenv("COLI_MOE_CPU_LANES", "lots", 1);
    coli_moe_lane_limits_from_env(&limits);
    CHECK(limits.cpu_lanes == 1, "a non-numeric lane count must fall back to 1");

    unsetenv("COLI_MOE_CPU_LANES");
    unsetenv("COLI_MOE_GPU_LANES");

    coli_moe_lane_limits_from_env(&limits);
    CHECK(coli_moe_lane_limit(&limits, COLI_ENGINE_NPU) == 1, "npu limit");
    CHECK(coli_moe_lane_limit(&limits, COLI_ENGINE_NONE) == 0, "none has no lanes");
    CHECK(coli_moe_lane_limit(NULL, COLI_ENGINE_CPU) == 0, "NULL limits");
}

/* ── Plan ── */

static bool kernel_for_enumerated_shapes(int rows, int inner, int out, int fmt,
                                         void *user) {
    (void)user;
    return fmt == 1 && coli_npu_shape_is_member(rows, inner, out);
}

static void build_caps(coli_placement_caps_t *caps) {
    coli_placement_caps_init(caps);
    caps->cpu_available = true;
    caps->gpu_available = true;
    caps->npu_available = true;
    caps->npu_resident_bytes = 64u * 1024u * 1024u;
    caps->npu_kernel_exists = kernel_for_enumerated_shapes;
}

/* A batch where one expert is hot (256 tokens) and two are a long tail. */
static int build_skewed_batch(coli_moe_assignment_t **out) {
    const int hot = 256, tail_a = 1, tail_b = 2;
    const int total = hot + tail_a + tail_b;
    coli_moe_assignment_t *a =
        (coli_moe_assignment_t *)calloc((size_t)total, sizeof(*a));
    if (!a) return -1;
    int n = 0;
    for (int i = 0; i < hot; ++i)    a[n++] = (coli_moe_assignment_t){ i, 7, 1.0f };
    for (int i = 0; i < tail_a; ++i) a[n++] = (coli_moe_assignment_t){ i, 3, 1.0f };
    for (int i = 0; i < tail_b; ++i) a[n++] = (coli_moe_assignment_t){ i, 9, 1.0f };
    *out = a;
    return total;
}

static void test_plan_splits_by_group_size(void) {
    coli_moe_assignment_t *assignments = NULL;
    const int count = build_skewed_batch(&assignments);
    CHECK(count > 0, "batch allocation failed");
    if (count <= 0) return;

    coli_moe_grouping_t grouping;
    coli_moe_grouping_init(&grouping);
    CHECK(coli_moe_group_by_expert(assignments, count, &grouping) == 0,
          "grouping failed");

    coli_placement_caps_t caps;
    build_caps(&caps);
    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));
    coli_moe_lane_limits_t limits;
    coli_moe_lane_limits_from_env(&limits);

    coli_moe_plan_t plan;
    coli_moe_plan_init(&plan);
    /* Expert gate_proj: [1024, 4096] -> inner 4096, out 1024. */
    CHECK(coli_moe_plan_build(&grouping, &policy, &caps, &limits,
                              4096, 1024, 1, &plan) == 0, "plan build failed");

    CHECK(plan.count == grouping.group_count, "plan should cover every group");
    CHECK(plan.unplaced == 0, "no group should be unplaceable here");

    for (int i = 0; i < plan.count; ++i) {
        const coli_moe_plan_item_t *item = &plan.items[i];
        CHECK(item->reason != NULL, "every placement must carry a reason");
        if (item->expert == 7) {
            CHECK(item->rows == 256, "the hot expert should have 256 rows");
            CHECK(item->engine == COLI_ENGINE_NPU,
                  "a 256-row expert group should reach the npu, got %s",
                  coli_engine_name(item->engine));
        } else {
            CHECK(item->rows <= 2, "tail expert %d has %d rows", item->expert,
                  item->rows);
            CHECK(item->engine == COLI_ENGINE_CPU,
                  "a 1-2 row tail group should stay on the cpu, got %s",
                  coli_engine_name(item->engine));
        }
        CHECK(item->lane >= 0 && item->lane < coli_moe_lane_limit(&limits, item->engine),
              "lane %d is outside the cap for %s", item->lane,
              coli_engine_name(item->engine));
    }

    CHECK(coli_moe_plan_engine_items(&plan, COLI_ENGINE_NPU) == 1,
          "exactly one group should be on the npu");
    CHECK(coli_moe_plan_engine_items(&plan, COLI_ENGINE_CPU) == 2,
          "the tail should be two cpu groups");

    coli_moe_plan_free(&plan);
    coli_moe_grouping_free(&grouping);
    free(assignments);
}

static void test_plan_lane_assignment(void) {
    /* Eight tail experts, all landing on the CPU, spread over four lanes. */
    coli_moe_assignment_t assignments[8];
    for (int i = 0; i < 8; ++i) {
        assignments[i] = (coli_moe_assignment_t){ i, i, 1.0f };
    }

    coli_moe_grouping_t grouping;
    coli_moe_grouping_init(&grouping);
    CHECK(coli_moe_group_by_expert(assignments, 8, &grouping) == 0, "grouping failed");

    coli_placement_caps_t caps;
    build_caps(&caps);
    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));

    coli_moe_lane_limits_t limits = { .cpu_lanes = 4, .gpu_lanes = 1,
                                      .npu_lanes = COLI_MOE_NPU_LANES };

    coli_moe_plan_t plan;
    coli_moe_plan_init(&plan);
    CHECK(coli_moe_plan_build(&grouping, &policy, &caps, &limits,
                              4096, 1024, 1, &plan) == 0, "plan build failed");

    int per_lane[4] = {0};
    for (int i = 0; i < plan.count; ++i) {
        CHECK(plan.items[i].engine == COLI_ENGINE_CPU,
              "single-row groups should be on the cpu");
        CHECK(plan.items[i].lane >= 0 && plan.items[i].lane < 4,
              "lane %d out of range", plan.items[i].lane);
        per_lane[plan.items[i].lane]++;
    }
    for (int l = 0; l < 4; ++l) {
        CHECK(per_lane[l] == 2, "lane %d got %d items, expected 2", l, per_lane[l]);
    }

    coli_moe_plan_free(&plan);
    coli_moe_grouping_free(&grouping);
}

static void test_plan_reports_unplaced(void) {
    coli_moe_assignment_t assignments[2] = {
        { 0, 1, 1.0f }, { 1, 1, 1.0f },
    };
    coli_moe_grouping_t grouping;
    coli_moe_grouping_init(&grouping);
    CHECK(coli_moe_group_by_expert(assignments, 2, &grouping) == 0, "grouping failed");

    /* No engine at all: the plan must say so rather than pick one. */
    coli_placement_caps_t caps;
    coli_placement_caps_init(&caps);
    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));
    coli_moe_lane_limits_t limits;
    coli_moe_lane_limits_from_env(&limits);

    coli_moe_plan_t plan;
    coli_moe_plan_init(&plan);
    CHECK(coli_moe_plan_build(&grouping, &policy, &caps, &limits,
                              4096, 1024, 1, &plan) == 0, "plan build failed");
    CHECK(plan.unplaced == 1, "the group should be reported unplaced");
    CHECK(plan.items[0].engine == COLI_ENGINE_NONE, "no engine may be invented");
    CHECK(plan.items[0].lane == -1, "an unplaced item has no lane");
    CHECK(plan.items[0].reason != NULL, "an unplaced item must explain itself");

    CHECK(coli_moe_plan_build(NULL, &policy, &caps, &limits, 4096, 1024, 1, &plan)
              == -EINVAL, "a NULL grouping must be rejected");
    CHECK(coli_moe_plan_build(&grouping, &policy, &caps, &limits, 0, 1024, 1, &plan)
              == -EINVAL, "a zero inner dimension must be rejected");

    coli_moe_plan_free(&plan);
    coli_moe_grouping_free(&grouping);
}


/* ── Execution ── */

typedef struct {
    int  calls;
    int  order[64];              /* group indices, in dispatch order */
    int  concurrent_npu;         /* largest npu width the executor issued */
    int  fail_on_group;          /* -1 to never fail */
    coli_engine_t seen[64];
} exec_probe_t;

static int exec_probe_dispatch(const coli_moe_plan_item_t *item, void *user) {
    exec_probe_t *probe = (exec_probe_t *)user;
    if (item->group_index == probe->fail_on_group) return -EIO;
    if (probe->calls < 64) {
        probe->order[probe->calls] = item->group_index;
        probe->seen[probe->calls] = item->engine;
    }
    probe->calls++;
    return 0;
}

/* Build a plan over `experts` single-token groups, all placed on `engine`. */
static int build_uniform_plan(coli_engine_t engine, int experts,
                              coli_moe_grouping_t *grouping,
                              coli_moe_plan_t *plan,
                              coli_moe_lane_limits_t *limits) {
    coli_moe_assignment_t *assignments =
        (coli_moe_assignment_t *)calloc((size_t)experts, sizeof(*assignments));
    if (!assignments) return -1;
    for (int i = 0; i < experts; ++i) {
        assignments[i] = (coli_moe_assignment_t){ i, i, 1.0f };
    }
    coli_moe_grouping_init(grouping);
    int ret = coli_moe_group_by_expert(assignments, experts, grouping);
    free(assignments);
    if (ret != 0) return -1;

    coli_placement_caps_t caps;
    coli_placement_caps_init(&caps);
    caps.cpu_available = (engine == COLI_ENGINE_CPU);
    caps.gpu_available = (engine == COLI_ENGINE_GPU);

    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));

    coli_moe_plan_init(plan);
    return coli_moe_plan_build(grouping, &policy, &caps, limits,
                               4096, 1024, 1, plan);
}

static void test_plan_execute(void) {
    coli_moe_lane_limits_t limits;
    coli_moe_lane_limits_from_env(&limits);
    limits.gpu_lanes = 2;

    coli_moe_grouping_t grouping;
    coli_moe_plan_t plan;
    CHECK(build_uniform_plan(COLI_ENGINE_GPU, 5, &grouping, &plan, &limits) == 0,
          "plan build failed");
    CHECK(plan.unplaced == 0, "every group should have been placed");

    exec_probe_t probe;
    memset(&probe, 0, sizeof(probe));
    probe.fail_on_group = -1;

    coli_moe_exec_stats_t stats;
    CHECK(coli_moe_plan_execute(&plan, &limits, exec_probe_dispatch, &probe, &stats)
              == 0, "execution failed");
    CHECK(probe.calls == 5, "every item must be dispatched exactly once, got %d",
          probe.calls);
    CHECK(stats.dispatched == 5, "stats disagree with the dispatcher");
    CHECK(stats.per_engine[COLI_ENGINE_GPU] == 5, "all five were gpu items");
    /* Two gpu lanes, five items: three waves of 2, 2, 1. */
    CHECK(stats.waves == 3, "expected 3 waves at 2 lanes, got %d", stats.waves);
    CHECK(stats.max_in_flight[COLI_ENGINE_GPU] == 2,
          "an engine must never be issued more than its lane count");
    /* Order is group order, which makes a run replayable. */
    for (int i = 0; i < 5; ++i) {
        CHECK(probe.order[i] == i, "item %d dispatched out of order (%d)",
              i, probe.order[i]);
    }

    coli_moe_plan_free(&plan);
    coli_moe_grouping_free(&grouping);
}

static void test_plan_execute_npu_is_serialised(void) {
    coli_moe_lane_limits_t limits;
    coli_moe_lane_limits_from_env(&limits);

    /* Hand-build a plan of npu items: one AIE partition means one hardware
     * context, so the executor must never issue two at once. */
    coli_moe_plan_item_t items[4];
    memset(items, 0, sizeof(items));
    for (int i = 0; i < 4; ++i) {
        items[i].group_index = i;
        items[i].expert = i;
        items[i].rows = 32;
        items[i].engine = COLI_ENGINE_NPU;
        items[i].lane = 0;
    }
    coli_moe_plan_t plan;
    coli_moe_plan_init(&plan);
    plan.items = items;
    plan.count = 4;
    plan.capacity = 4;

    exec_probe_t probe;
    memset(&probe, 0, sizeof(probe));
    probe.fail_on_group = -1;

    coli_moe_exec_stats_t stats;
    CHECK(coli_moe_plan_execute(&plan, &limits, exec_probe_dispatch, &probe, &stats)
              == 0, "npu execution failed");
    CHECK(stats.max_in_flight[COLI_ENGINE_NPU] == 1,
          "the npu must be issued one at a time, got %d",
          stats.max_in_flight[COLI_ENGINE_NPU]);
    CHECK(stats.waves == 4, "four serialised items need four waves, got %d",
          stats.waves);
    /* plan.items is stack memory here; do not free it through the plan. */
}

static void test_plan_execute_refusals(void) {
    coli_moe_lane_limits_t limits;
    coli_moe_lane_limits_from_env(&limits);

    coli_moe_grouping_t grouping;
    coli_moe_plan_t plan;
    CHECK(build_uniform_plan(COLI_ENGINE_CPU, 3, &grouping, &plan, &limits) == 0,
          "plan build failed");

    exec_probe_t probe;
    memset(&probe, 0, sizeof(probe));
    probe.fail_on_group = 1;

    coli_moe_exec_stats_t stats;
    /* A failing dispatch stops the run and is reported; it is never re-placed
     * onto another engine. */
    CHECK(coli_moe_plan_execute(&plan, &limits, exec_probe_dispatch, &probe, &stats)
              == -EIO, "a failing dispatch must surface its error");
    CHECK(stats.failed_item == 1, "the failing item must be identified, got %d",
          stats.failed_item);
    CHECK(stats.failed_rc == -EIO, "the dispatcher's return value must survive");
    CHECK(probe.calls == 1, "nothing may be dispatched after a failure");

    /* A plan that could not place everything is not runnable. */
    plan.unplaced = 1;
    memset(&probe, 0, sizeof(probe));
    probe.fail_on_group = -1;
    CHECK(coli_moe_plan_execute(&plan, &limits, exec_probe_dispatch, &probe, NULL)
              == -EINVAL, "an unplaceable plan must be refused");
    CHECK(probe.calls == 0, "a refused plan must dispatch nothing");
    plan.unplaced = 0;

    CHECK(coli_moe_plan_execute(NULL, &limits, exec_probe_dispatch, &probe, NULL)
              == -EINVAL, "a NULL plan must be refused");
    CHECK(coli_moe_plan_execute(&plan, &limits, NULL, &probe, NULL) == -EINVAL,
          "a NULL dispatcher must be refused");

    coli_moe_plan_free(&plan);
    coli_moe_grouping_free(&grouping);
}

/* ── Residency as a placement input (A4.3) ── */

static bool resident_on_gpu_only(int expert, coli_engine_t engine, void *user) {
    (void)user;
    return engine == COLI_ENGINE_GPU && expert == 7;
}

static void test_plan_build_residency_input(void) {
    coli_moe_assignment_t assignments[4] = {
        { 0, 7, 1.0f }, { 1, 7, 1.0f }, { 2, 9, 1.0f }, { 3, 9, 1.0f },
    };
    coli_moe_grouping_t grouping;
    coli_moe_grouping_init(&grouping);
    CHECK(coli_moe_group_by_expert(assignments, 4, &grouping) == 0, "grouping failed");

    /* The gpu is measurably slower cold and faster warm; only the residency
     * input can tell the two groups apart, since their shapes are identical. */
    const char *table =
        "cpu,2,4096,1024,1,5,9000.0,0.0,-1,-1,-1,-1,-1,-1\n"
        "gpu,2,4096,1024,1,5,12000.0,2000.0,500,6000,500,500,500,0\n";
    coli_shape_profile_t *profile = coli_profile_create();
    CHECK(profile != NULL, "profile allocation failed");
    if (!profile) { coli_moe_grouping_free(&grouping); return; }
    FILE *fp = tmpfile();
    CHECK(fp != NULL, "tmpfile failed");
    if (!fp) { coli_profile_free(profile); coli_moe_grouping_free(&grouping); return; }
    fputs(table, fp);
    rewind(fp);
    CHECK(coli_profile_parse(profile, fp, NULL) == 0, "table did not parse");
    fclose(fp);

    coli_placement_caps_t caps;
    coli_placement_caps_init(&caps);
    caps.cpu_available = true;
    caps.gpu_available = true;

    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));
    policy.profile = profile;

    coli_moe_lane_limits_t limits;
    coli_moe_lane_limits_from_env(&limits);

    coli_moe_plan_t cold;
    coli_moe_plan_init(&cold);
    CHECK(coli_moe_plan_build(&grouping, &policy, &caps, &limits,
                              4096, 1024, 1, &cold) == 0, "cold plan failed");
    CHECK(cold.items[0].engine == COLI_ENGINE_CPU,
          "cold: the cpu record is cheaper, got %s",
          coli_engine_name(cold.items[0].engine));
    coli_moe_plan_free(&cold);

    coli_moe_plan_t warm;
    coli_moe_plan_init(&warm);
    CHECK(coli_moe_plan_build_resident(&grouping, &policy, &caps, &limits,
                                       4096, 1024, 1, resident_on_gpu_only, NULL,
                                       &warm) == 0, "warm plan failed");
    CHECK(warm.count == 2, "two experts, two groups");
    CHECK(warm.items[0].expert == 7 && warm.items[0].engine == COLI_ENGINE_GPU,
          "a warm expert must beat a cold cpu, got %s",
          coli_engine_name(warm.items[0].engine));
    CHECK(warm.items[1].expert == 9 && warm.items[1].engine == COLI_ENGINE_CPU,
          "a cold expert of the same shape must still go to the cpu, got %s",
          coli_engine_name(warm.items[1].engine));
    coli_moe_plan_free(&warm);

    coli_profile_free(profile);
    coli_moe_grouping_free(&grouping);
}

/* ── Residency ── */

static void test_residency(void) {
    /* One Qwen 3.6 expert projection: 1024x4096 int8 = 4 MiB. */
    const size_t expert_bytes = 1024u * 4096u;
    coli_moe_residency_t *residency =
        coli_moe_residency_create(3 * expert_bytes, 8);
    CHECK(residency != NULL, "residency creation failed");
    if (!residency) return;

    bool resident = false;
    int evicted[4];
    int evicted_count = 0;

    for (int e = 0; e < 3; ++e) {
        CHECK(coli_moe_residency_touch(residency, e, expert_bytes, &resident,
                                       evicted, 4, &evicted_count) == 0,
              "touch of expert %d failed", e);
        CHECK(!resident, "expert %d should be a miss on first use", e);
        CHECK(evicted_count == 0, "no eviction should be needed yet");
    }
    CHECK(coli_moe_residency_count(residency) == 3, "three experts should be held");
    CHECK(coli_moe_residency_bytes(residency) == 3 * expert_bytes, "byte accounting");

    /* A repeat use is a hit: no upload, and it becomes the most recent. */
    CHECK(coli_moe_residency_touch(residency, 0, expert_bytes, &resident,
                                   evicted, 4, &evicted_count) == 0, "re-touch failed");
    CHECK(resident, "expert 0 should be resident");
    CHECK(evicted_count == 0, "a hit must not evict");

    /* Budget full: the least recently used (expert 1) is evicted, not expert 0. */
    CHECK(coli_moe_residency_touch(residency, 3, expert_bytes, &resident,
                                   evicted, 4, &evicted_count) == 0, "touch 3 failed");
    CHECK(!resident, "expert 3 is new");
    CHECK(evicted_count == 1, "one eviction expected, got %d", evicted_count);
    CHECK(evicted[0] == 1, "the LRU victim should be expert 1, got %d", evicted[0]);
    CHECK(coli_moe_residency_contains(residency, 0), "expert 0 must survive");
    CHECK(!coli_moe_residency_contains(residency, 1), "expert 1 must be gone");
    CHECK(coli_moe_residency_bytes(residency) == 3 * expert_bytes,
          "byte accounting after eviction");

    /* An expert larger than the whole budget is a configuration error. */
    CHECK(coli_moe_residency_touch(residency, 4, 100u * expert_bytes, &resident,
                                   evicted, 4, &evicted_count) == -ENOSPC,
          "an oversized expert must be refused");
    CHECK(coli_moe_residency_contains(residency, 0),
          "a refused touch must not have evicted anything");

    CHECK(coli_moe_residency_touch(residency, -1, expert_bytes, &resident,
                                   evicted, 4, &evicted_count) == -EINVAL,
          "a negative expert id must be rejected");
    CHECK(coli_moe_residency_touch(NULL, 0, expert_bytes, &resident,
                                   evicted, 4, &evicted_count) == -EINVAL,
          "a NULL residency must be rejected");

    coli_moe_residency_free(residency);

    CHECK(coli_moe_residency_create(0, 8) == NULL, "a zero budget is invalid");
    CHECK(coli_moe_residency_create(1024, 0) == NULL, "zero slots is invalid");
    CHECK(coli_moe_residency_bytes(NULL) == 0, "NULL residency holds nothing");
}

static void test_residency_slot_pressure(void) {
    /* Budget is generous, but only two slots: entry count must be respected. */
    coli_moe_residency_t *residency = coli_moe_residency_create(1u << 30, 2);
    CHECK(residency != NULL, "creation failed");
    if (!residency) return;

    bool resident = false;
    int evicted[2];
    int evicted_count = 0;

    for (int e = 0; e < 2; ++e) {
        CHECK(coli_moe_residency_touch(residency, e, 4096, &resident,
                                       evicted, 2, &evicted_count) == 0,
              "touch %d failed", e);
    }
    CHECK(coli_moe_residency_count(residency) == 2, "two slots should be used");

    CHECK(coli_moe_residency_touch(residency, 2, 4096, &resident,
                                   evicted, 2, &evicted_count) == 0, "touch 2 failed");
    CHECK(evicted_count == 1, "slot pressure should evict one, got %d", evicted_count);
    CHECK(evicted[0] == 0, "the LRU victim should be expert 0");
    CHECK(coli_moe_residency_count(residency) == 2, "still two slots");

    coli_moe_residency_free(residency);
}

int main(void) {
    printf("=== MoE scheduling tests ===\n");

    test_grouping_basic();
    test_grouping_edges();
    test_lane_limits();
    test_plan_splits_by_group_size();
    test_plan_lane_assignment();
    test_plan_reports_unplaced();
    test_plan_execute();
    test_plan_execute_npu_is_serialised();
    test_plan_execute_refusals();
    test_plan_build_residency_input();
    test_residency();
    test_residency_slot_pressure();

    if (g_failures > 0) {
        printf("%d MoE scheduling check(s) FAILED\n", g_failures);
        return 1;
    }
    printf("MoE scheduling tests passed\n");
    return 0;
}
