/**
 * test_placement.c — host-side tests for the profile table, the Qwen 3.6 MoE
 * shape set and the placement policy.
 *
 * Every test here is pure host arithmetic and runs on any machine. Nothing in
 * this file touches a device: that is deliberate, because the decision logic is
 * exactly the part that must be verifiable without the silicon. The tests that
 * do need Strix Halo live in tests/npu_device_test.c and report SKIP.
 */

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sched/backend_placement.h"
#include "sched/npu_shapes.h"
#include "sched/shape_profile.h"

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

/* ── Shape set ── */

static void test_shape_set(void) {
    size_t projections = 0, tiles = 0, shapes = 0;
    const coli_npu_projection_t *p = coli_npu_projections(&projections);
    const int *t = coli_npu_row_tiles(&tiles);
    const coli_npu_shape_t *s = coli_npu_shape_set(&shapes);

    CHECK(p && projections == 5, "expected 5 projections, got %zu", projections);
    CHECK(t && tiles == 3, "expected 3 row tiles, got %zu", tiles);
    CHECK(s && shapes == projections * tiles,
          "shape set should be the cross product, got %zu", shapes);

    /* The tiles must be ordered largest first: the tiler is greedy. */
    for (size_t i = 1; i < tiles; ++i) {
        CHECK(t[i] < t[i - 1], "row tiles are not descending at %zu", i);
    }
    CHECK(t[tiles - 1] == 1, "the smallest row tile must be 1 for exact tiling");

    /* Every shape in the set must be recognised as a member. */
    for (size_t i = 0; i < shapes; ++i) {
        CHECK(coli_npu_shape_is_member(s[i].rows, s[i].inner, s[i].out),
              "shape %dx%dx%d is in the set but not a member",
              s[i].rows, s[i].inner, s[i].out);
    }

    /* The model's real projections. */
    CHECK(coli_npu_projection_is_known(4096, 1024), "expert gate/up missing");
    CHECK(coli_npu_projection_is_known(1024, 4096), "expert down missing");
    CHECK(coli_npu_projection_is_known(4096, 16384), "q_proj missing");
    CHECK(coli_npu_projection_is_known(4096, 512), "k/v_proj and router missing");
    CHECK(coli_npu_projection_is_known(8192, 4096), "o_proj missing");

    /* And nothing else: no widening. */
    CHECK(!coli_npu_projection_is_known(4097, 1024), "off-by-one inner accepted");
    CHECK(!coli_npu_projection_is_known(4096, 1025), "off-by-one out accepted");
    CHECK(!coli_npu_shape_is_member(2, 4096, 1024), "rows=2 is not a tile");

    char name[64];
    int n = coli_npu_shape_filename(32, 4096, 1024, name, sizeof(name));
    CHECK(n > 0 && strcmp(name, "matmul_int8_32x4096x1024.npukernel") == 0,
          "unexpected artifact name '%s'", name);
    CHECK(coli_npu_shape_filename(32, 4096, 1024, name, 4) == -ENOSPC,
          "a short buffer must be refused, not truncated");
}

/* ── Tiling ── */

static void check_tiling(int rows) {
    coli_npu_tiling_t plan;
    int ret = coli_npu_plan_tiles(rows, 4096, 1024, &plan);
    CHECK(ret == 0, "tiling %d rows failed with %d", rows, ret);
    if (ret != 0) return;

    CHECK(plan.complete, "tiling of %d rows is incomplete", rows);
    CHECK(plan.padded_rows == rows,
          "tiling of %d rows covers %d rows", rows, plan.padded_rows);

    int covered = 0;
    for (int i = 0; i < plan.count; ++i) {
        CHECK(plan.tiles[i].row_offset == covered,
              "tile %d of %d rows starts at %d, expected %d",
              i, rows, plan.tiles[i].row_offset, covered);
        CHECK(coli_npu_shape_is_member(plan.tiles[i].rows, 4096, 1024),
              "tile %d of %d rows has non-member size %d",
              i, rows, plan.tiles[i].rows);
        covered += plan.tiles[i].rows;
    }
    CHECK(covered == rows, "tiles cover %d of %d rows", covered, rows);
}

static void test_tiling(void) {
    const int cases[] = { 1, 2, 7, 31, 32, 33, 255, 256, 257, 1000 };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        check_tiling(cases[i]);
    }

    /* Greedy over {256, 32, 1}: 300 = 256 + 32 + 12x1. */
    coli_npu_tiling_t plan;
    CHECK(coli_npu_plan_tiles(300, 4096, 1024, &plan) == 0, "300 rows failed");
    CHECK(plan.count == 14, "expected 14 tiles for 300 rows, got %d", plan.count);
    CHECK(plan.tiles[0].rows == 256, "first tile should be the largest");

    /* An unknown projection is refused outright — no nearest match. */
    CHECK(coli_npu_plan_tiles(32, 4096, 999, &plan) == -ENOENT,
          "an unknown projection must be refused");
    CHECK(coli_npu_plan_tiles(0, 4096, 1024, &plan) == -EINVAL,
          "zero rows must be rejected");

    /* Far more rows than the tile budget can express. */
    CHECK(coli_npu_plan_tiles(1 << 20, 4096, 1024, &plan) == -E2BIG,
          "an unbounded row count must be refused, not silently truncated");

    CHECK(coli_npu_operand_bytes(1, 4096, 1024) ==
              (size_t)4096 + (size_t)1024 * 4096 + 1024 * sizeof(float),
          "operand byte accounting is wrong");
    CHECK(coli_npu_operand_bytes(0, 4096, 1024) == 0, "zero rows must be 0 bytes");
}

/* ── Profile table ── */

static const char *k_table =
    "backend,rows,inner,out,fmt,iters,total_ns,fixed_ns,alloc_ns,upload_ns,"
    "submit_ns,wait_ns,readback_ns,teardown_ns\n"
    "# a comment\n"
    "cpu,1,4096,1024,1,5,1000.0,0.0,-1,-1,-1,-1,-1,-1\n"
    "npu,1,4096,1024,1,5,9000.0,8000.0,3000,2000,500,1000,1500,1000\n"
    "npu,256,4096,1024,1,5,20000.0,8000.0,3000,2000,500,12000,1500,1000\n"
    "gpu,256,4096,1024,1,5,15000.0,-1,-1,-1,-1,-1,-1,-1\n";

static coli_shape_profile_t *load_table(const char *text) {
    coli_shape_profile_t *profile = coli_profile_create();
    if (!profile) return NULL;
    FILE *fp = tmpfile();
    if (!fp) {
        coli_profile_free(profile);
        return NULL;
    }
    fputs(text, fp);
    rewind(fp);
    int error_line = 0;
    int ret = coli_profile_parse(profile, fp, &error_line);
    fclose(fp);
    if (ret != 0) {
        fprintf(stderr, "table parse failed: %d at line %d\n", ret, error_line);
        coli_profile_free(profile);
        return NULL;
    }
    return profile;
}

static void test_profile_parse(void) {
    coli_shape_profile_t *profile = load_table(k_table);
    CHECK(profile != NULL, "the sample table did not parse");
    if (!profile) return;

    CHECK(coli_profile_count(profile) == 4,
          "expected 4 records, got %zu", coli_profile_count(profile));

    const coli_shape_record_t *first = coli_profile_at(profile, 0);
    CHECK(first && first->engine == COLI_ENGINE_CPU, "first record is not cpu");
    CHECK(first && first->rows == 1 && first->inner == 4096 && first->out == 1024,
          "first record has the wrong shape");
    CHECK(coli_profile_at(profile, 99) == NULL, "out-of-range index must be NULL");

    double ns = 0.0;
    CHECK(coli_profile_estimate_ns(profile, COLI_ENGINE_CPU, 1, 4096, 1024, 1, &ns)
              == COLI_PROFILE_EXACT, "exact cpu lookup failed");
    CHECK(fabs(ns - 1000.0) < 1e-6, "exact lookup returned %f", ns);

    /* Estimate: fixed cost kept, array time scaled by the MAC ratio. The
     * 256-row npu record has 12000 ns of array time, so 128 rows is 6000. */
    CHECK(coli_profile_estimate_ns(profile, COLI_ENGINE_NPU, 128, 4096, 1024, 1, &ns)
              == COLI_PROFILE_ESTIMATE, "npu estimate failed");
    CHECK(fabs(ns - (8000.0 + 6000.0)) < 1.0, "npu estimate returned %f", ns);

    /* Nothing measured for that engine and format. */
    CHECK(coli_profile_estimate_ns(profile, COLI_ENGINE_GPU, 1, 4096, 1024, 7, &ns)
              == COLI_PROFILE_MISS, "an unmeasured fmt must miss");

    coli_profile_free(profile);
}

static void test_profile_rejects_garbage(void) {    const char *bad[] = {
        "cpu,1,4096,1024,1,5,1000.0\n",                          /* short row */
        "tpu,1,4096,1024,1,5,1,0,-1,-1,-1,-1,-1,-1\n",           /* unknown engine */
        "cpu,0,4096,1024,1,5,1,0,-1,-1,-1,-1,-1,-1\n",           /* zero rows */
        "cpu,1,4096,1024,1,0,1,0,-1,-1,-1,-1,-1,-1\n",           /* zero iters */
        "cpu,x,4096,1024,1,5,1,0,-1,-1,-1,-1,-1,-1\n",           /* non-numeric */
    };
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        coli_shape_profile_t *profile = coli_profile_create();
        CHECK(profile != NULL, "allocation failed");
        if (!profile) return;
        FILE *fp = tmpfile();
        CHECK(fp != NULL, "tmpfile failed");
        if (!fp) { coli_profile_free(profile); return; }
        fputs(bad[i], fp);
        rewind(fp);
        int line = 0;
        int ret = coli_profile_parse(profile, fp, &line);
        CHECK(ret == -EINVAL, "malformed table %zu parsed with %d", i, ret);
        CHECK(line == 1, "error line for table %zu was %d", i, line);
        fclose(fp);
        coli_profile_free(profile);
    }
}

static void test_profile_roundtrip(void) {
    coli_shape_profile_t *profile = load_table(k_table);
    CHECK(profile != NULL, "table did not load");
    if (!profile) return;

    FILE *fp = tmpfile();
    CHECK(fp != NULL, "tmpfile failed");
    if (!fp) { coli_profile_free(profile); return; }
    CHECK(coli_profile_write(profile, fp) == 0, "write failed");
    rewind(fp);

    coli_shape_profile_t *again = coli_profile_create();
    CHECK(again != NULL, "allocation failed");
    if (again) {
        int line = 0;
        CHECK(coli_profile_parse(again, fp, &line) == 0,
              "re-parse failed at line %d", line);
        CHECK(coli_profile_count(again) == coli_profile_count(profile),
              "round trip lost records");
        coli_profile_free(again);
    }
    fclose(fp);
    coli_profile_free(profile);
}

/* ── Placement ── */

static bool kernel_for_enumerated_shapes(int rows, int inner, int out, int fmt,
                                         void *user) {
    (void)user;
    return fmt == 1 && coli_npu_shape_is_member(rows, inner, out);
}

static bool kernel_never(int rows, int inner, int out, int fmt, void *user) {
    (void)rows; (void)inner; (void)out; (void)fmt; (void)user;
    return false;
}

static void all_available(coli_placement_caps_t *caps) {
    coli_placement_caps_init(caps);
    caps->cpu_available = true;
    caps->gpu_available = true;
    caps->npu_available = true;
    caps->npu_resident_bytes = 64u * 1024u * 1024u; /* the amdxdna heap cap */
    caps->npu_kernel_exists = kernel_for_enumerated_shapes;
}

static void test_placement_constraints(void) {
    coli_placement_caps_t caps;
    all_available(&caps);

    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));

    coli_placement_decision_t decision;

    /* No kernel artifact: the NPU is never a candidate, whatever the size. */
    caps.npu_kernel_exists = kernel_never;
    coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1, &decision);
    CHECK(decision.engine != COLI_ENGINE_NPU,
          "the NPU was chosen with no kernel artifact");
    CHECK(decision.rejected[COLI_ENGINE_NPU] != NULL,
          "a refused NPU must carry a reason");

    /* A shape outside the enumerated set has no artifact either. */
    all_available(&caps);
    coli_choose_backend(&policy, &caps, 7, 4096, 1024, 1, &decision);
    CHECK(decision.engine != COLI_ENGINE_NPU,
          "rows=7 is not a tile and must not reach the NPU");

    /* Residency: a 256x4096x16384 dispatch needs ~70 MiB, over the heap cap. */
    all_available(&caps);
    coli_choose_backend(&policy, &caps, 256, 4096, 16384, 1, &decision);
    CHECK(decision.rejected[COLI_ENGINE_NPU] != NULL,
          "an oversized operand set must be refused by the residency check");

    /* Everything unavailable: no engine, and a reason. */
    coli_placement_caps_init(&caps);
    coli_engine_t engine = coli_choose_backend(&policy, &caps, 32, 4096, 1024, 1,
                                               &decision);
    CHECK(engine == COLI_ENGINE_NONE, "an engine was invented from nothing");
    CHECK(decision.reason != NULL, "a refusal must explain itself");

    /* Degenerate shapes. */
    all_available(&caps);
    CHECK(coli_choose_backend(&policy, &caps, 0, 4096, 1024, 1, &decision)
              == COLI_ENGINE_NONE, "zero rows must be refused");
    CHECK(coli_choose_backend(NULL, &caps, 1, 1, 1, 1, &decision)
              == COLI_ENGINE_NONE, "a NULL policy must be refused");
}

/* ── Row-tile class guard (A4.4) ── */

static void test_row_class_guard(void) {
    /* Only a decode record for the cpu, only a prefill record for the gpu. */
    const char *table =
        "cpu,1,4096,1024,1,5,1000.0,0.0,-1,-1,-1,-1,-1,-1\n"
        "gpu,256,4096,1024,1,5,15000.0,2000.0,-1,-1,-1,-1,-1,-1\n";
    coli_shape_profile_t *profile = load_table(table);
    CHECK(profile != NULL, "table did not load");
    if (!profile) return;

    CHECK(coli_row_class(1) == COLI_ROW_CLASS_DECODE, "rows=1 is decode");
    CHECK(coli_row_class(32) == COLI_ROW_CLASS_PREFILL, "rows=32 is prefill");
    CHECK(strcmp(coli_row_class_name(COLI_ROW_CLASS_PREFILL), "prefill") == 0,
          "prefill class name");

    double ns = 0.0;
    /* A prefill request must not be estimated from the decode-only cpu record:
     * that is the extrapolation where fixed cost dominance flips. */
    CHECK(coli_profile_estimate_ns(profile, COLI_ENGINE_CPU, 256, 4096, 1024, 1, &ns)
              == COLI_PROFILE_MISS,
          "a prefill shape was estimated from a decode record");
    /* And the mirror: decode must not be estimated from the prefill gpu record. */
    CHECK(coli_profile_estimate_ns(profile, COLI_ENGINE_GPU, 1, 4096, 1024, 1, &ns)
              == COLI_PROFILE_MISS,
          "a decode shape was estimated from a prefill record");
    /* Within a class it still estimates. */
    CHECK(coli_profile_estimate_ns(profile, COLI_ENGINE_GPU, 128, 4096, 1024, 1, &ns)
              == COLI_PROFILE_ESTIMATE, "a same-class estimate must still work");
    /* An exact record is an exact record regardless of class filtering. */
    CHECK(coli_profile_estimate_ns(profile, COLI_ENGINE_CPU, 1, 4096, 1024, 1, &ns)
              == COLI_PROFILE_EXACT, "the exact record was lost");

    coli_profile_free(profile);
}

/* ── Data-movement term (A4.3) ── */

static void test_upload_cost(void) {
    /* One npu record whose upload stage is attributed, one gpu record whose is
     * not. 256x4096x1024 moves 256*4096 + 4096*1024 + 256*1024*4 = 6291456
     * bytes, of which 4194304 are weights: two thirds of the 3000 ns upload. */
    const char *table =
        "npu,256,4096,1024,1,5,20000.0,8000.0,1000,3000,500,12000,1500,1000\n"
        "gpu,256,4096,1024,1,5,15000.0,-1,-1,-1,-1,-1,-1,-1\n";
    coli_shape_profile_t *profile = load_table(table);
    CHECK(profile != NULL, "table did not load");
    if (!profile) return;

    double ns = -1.0;
    CHECK(coli_profile_upload_ns(profile, COLI_ENGINE_NPU, 256, 4096, 1024, 1, &ns)
              == COLI_PROFILE_EXACT, "upload lookup failed");
    CHECK(fabs(ns - 2000.0) < 1.0, "weight-upload share was %f, expected 2000", ns);

    /* A record that did not attribute an upload stage yields no saving: an
     * unmeasured stage must never become a discount. */
    ns = -1.0;
    CHECK(coli_profile_upload_ns(profile, COLI_ENGINE_GPU, 256, 4096, 1024, 1, &ns)
              == COLI_PROFILE_EXACT, "gpu upload lookup failed");
    CHECK(ns == 0.0, "an unmeasured upload stage reported %f", ns);

    /* Placement must charge the cold engine and credit the warm one. */
    coli_placement_caps_t caps;
    all_available(&caps);
    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));
    policy.profile = profile;

    coli_placement_decision_t cold, warm;
    coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1, &cold);
    CHECK(cold.engine == COLI_ENGINE_GPU,
          "cold: the 15000 ns gpu record should beat the 20000 ns npu one, got %s",
          coli_engine_name(cold.engine));
    CHECK(fabs(cold.upload_ns[COLI_ENGINE_NPU] - 2000.0) < 1.0,
          "the decision must report the measured upload share");

    caps.weights_resident[COLI_ENGINE_NPU] = true;
    coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1, &warm);
    CHECK(fabs(warm.estimate_ns[COLI_ENGINE_NPU] - 18000.0) < 1.0,
          "a warm npu must be charged 20000-2000 ns, got %f",
          warm.estimate_ns[COLI_ENGINE_NPU]);
    CHECK(fabs(warm.estimate_ns[COLI_ENGINE_GPU]
               - cold.estimate_ns[COLI_ENGINE_GPU]) < 1e-9,
          "an engine that is not resident must be unaffected");
    /* Warm still does not beat the gpu here (18000 > 15000) — the point is that
     * the number moved by exactly the measured amount and no more. */

    coli_profile_free(profile);
}

/* ── Permanent refusals (A1.4) and the iGPU budget (A4.2) ── */

static void test_permanent_refusal_and_gpu_budget(void) {
    coli_placement_caps_t caps;
    all_available(&caps);

    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));
    coli_placement_decision_t decision;

    /* rows=1 is a property of the AIE2P MAC, not of this build: it must be
     * refused even with a kernel callback that claims an artifact exists, and
     * it must be marked permanent so a report does not tell the operator to
     * wait for a kernel that can never be built. */
    coli_choose_backend(&policy, &caps, 1, 4096, 1024, 1, &decision);
    CHECK(decision.engine != COLI_ENGINE_NPU, "rows=1 must never reach the NPU");
    CHECK(decision.rejected_permanent[COLI_ENGINE_NPU],
          "the rows=1 refusal must be marked permanent");

    /* A missing artifact is a different, non-permanent refusal. */
    all_available(&caps);
    caps.npu_kernel_exists = kernel_never;
    coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1, &decision);
    CHECK(decision.rejected[COLI_ENGINE_NPU] != NULL, "a refusal needs a reason");
    CHECK(!decision.rejected_permanent[COLI_ENGINE_NPU],
          "a missing artifact is not a permanent refusal");

    /* The iGPU budget is honoured when known and ignored when zero. */
    all_available(&caps);
    caps.gpu_resident_bytes = 1024;
    coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1, &decision);
    CHECK(decision.rejected[COLI_ENGINE_GPU] != NULL,
          "an operand set over the gpu budget must be refused");
    caps.gpu_resident_bytes = 0;
    coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1, &decision);
    CHECK(decision.rejected[COLI_ENGINE_GPU] == NULL,
          "an unknown gpu budget must not act as a constraint");
}

static void test_placement_forced(void) {
    coli_placement_caps_t caps;
    all_available(&caps);

    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));
    policy.forced = COLI_ENGINE_NPU;

    coli_placement_decision_t decision;
    coli_engine_t engine = coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1,
                                               &decision);
    CHECK(engine == COLI_ENGINE_NPU, "a satisfiable pin must be honoured");
    CHECK(decision.source == COLI_PLACEMENT_SOURCE_FORCED,
          "a pinned decision must be labelled forced");

    /* Pinned to an engine that cannot run it: refuse, never substitute. */
    caps.npu_kernel_exists = kernel_never;
    engine = coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1, &decision);
    CHECK(engine == COLI_ENGINE_NONE,
          "a pinned engine that cannot run the shape must not be replaced");
    CHECK(decision.reason != NULL && strstr(decision.reason, "npukernel") != NULL,
          "the refusal must name the constraint, got '%s'",
          decision.reason ? decision.reason : "(null)");
}

static void test_placement_measured(void) {
    coli_shape_profile_t *profile = load_table(k_table);
    CHECK(profile != NULL, "table did not load");
    if (!profile) return;

    coli_placement_caps_t caps;
    all_available(&caps);

    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));
    policy.profile = profile;

    coli_placement_decision_t decision;

    /* Decode: the table says cpu 1000 ns vs npu 9000 ns. The measurement, not
     * the structural guess, must decide. */
    coli_engine_t engine = coli_choose_backend(&policy, &caps, 1, 4096, 1024, 1,
                                               &decision);
    CHECK(engine == COLI_ENGINE_CPU, "measured decode should pick the cpu, got %s",
          coli_engine_name(engine));
    CHECK(decision.source == COLI_PLACEMENT_SOURCE_MEASURED,
          "an exact record must be labelled measured");

    /* Bulk prefill: npu 20000 ns beats gpu's 15000? No — gpu is cheaper here,
     * and the policy must follow the table rather than a preference for the
     * accelerator. */
    engine = coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1, &decision);
    CHECK(engine == COLI_ENGINE_GPU,
          "measured bulk prefill should follow the table, got %s",
          coli_engine_name(engine));

    /* A shape with no record for any engine falls back to an estimate. */
    engine = coli_choose_backend(&policy, &caps, 32, 4096, 1024, 1, &decision);
    CHECK(engine != COLI_ENGINE_NONE, "an estimate should still decide");
    CHECK(decision.source == COLI_PLACEMENT_SOURCE_ESTIMATED,
          "a scaled decision must be labelled estimated, got %s",
          coli_placement_source_name(decision.source));

    coli_profile_free(profile);
}

static void test_placement_requires_profile(void) {
    coli_placement_caps_t caps;
    all_available(&caps);

    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));
    policy.require_profile = true;

    coli_placement_decision_t decision;
    coli_engine_t engine = coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1,
                                               &decision);
    CHECK(engine == COLI_ENGINE_NONE,
          "require_profile must refuse a structural decision");
    CHECK(decision.reason && strstr(decision.reason, "bench") != NULL,
          "the refusal should point at the benchmark, got '%s'",
          decision.reason ? decision.reason : "(null)");

    /* Without the flag the same call decides, and says it is structural. */
    policy.require_profile = false;
    engine = coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1, &decision);
    CHECK(engine != COLI_ENGINE_NONE, "a structural decision should still be made");
    CHECK(decision.source == COLI_PLACEMENT_SOURCE_STRUCTURAL,
          "an unmeasured decision must be labelled structural");
}

static void test_placement_structural_shape_sensitivity(void) {
    coli_placement_caps_t caps;
    all_available(&caps);

    coli_placement_policy_t policy;
    memset(&policy, 0, sizeof(policy));

    coli_placement_decision_t decision;

    /* rows=1 reuses each weight byte once: the device round trip cannot pay. */
    coli_engine_t decode = coli_choose_backend(&policy, &caps, 1, 4096, 1024, 1,
                                               &decision);
    CHECK(decode == COLI_ENGINE_CPU,
          "structural decode should stay on the cpu, got %s",
          coli_engine_name(decode));

    /* rows=256 reuses each weight byte 256 times. */
    coli_engine_t bulk = coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1,
                                             &decision);
    CHECK(bulk == COLI_ENGINE_NPU,
          "structural bulk prefill should reach the npu, got %s",
          coli_engine_name(bulk));

    /* With no NPU kernel the same bulk shape goes to the iGPU, not the CPU. */
    caps.npu_kernel_exists = kernel_never;
    bulk = coli_choose_backend(&policy, &caps, 256, 4096, 1024, 1, &decision);
    CHECK(bulk == COLI_ENGINE_GPU,
          "bulk prefill without an artifact should go to the igpu, got %s",
          coli_engine_name(bulk));
}

static void test_policy_from_env(void) {
    coli_placement_policy_t policy;

    unsetenv("COLI_PLACEMENT");
    unsetenv("COLI_PLACEMENT_REQUIRE_PROFILE");
    CHECK(coli_placement_policy_from_env(&policy, NULL) == 0, "default env failed");
    CHECK(policy.forced == COLI_ENGINE_NONE, "default must not pin an engine");
    CHECK(!policy.require_profile, "default must not require a profile");

    setenv("COLI_PLACEMENT", "npu", 1);
    CHECK(coli_placement_policy_from_env(&policy, NULL) == 0, "npu env failed");
    CHECK(policy.forced == COLI_ENGINE_NPU, "COLI_PLACEMENT=npu was not honoured");

    setenv("COLI_PLACEMENT", "auto", 1);
    CHECK(coli_placement_policy_from_env(&policy, NULL) == 0, "auto env failed");
    CHECK(policy.forced == COLI_ENGINE_NONE, "auto must not pin an engine");

    setenv("COLI_PLACEMENT", "banana", 1);
    CHECK(coli_placement_policy_from_env(&policy, NULL) == -EINVAL,
          "an unknown engine name must be rejected, not ignored");
    unsetenv("COLI_PLACEMENT");

    setenv("COLI_PLACEMENT_REQUIRE_PROFILE", "1", 1);
    CHECK(coli_placement_policy_from_env(&policy, NULL) == 0, "require env failed");
    CHECK(policy.require_profile, "COLI_PLACEMENT_REQUIRE_PROFILE=1 ignored");
    setenv("COLI_PLACEMENT_REQUIRE_PROFILE", "0", 1);
    CHECK(coli_placement_policy_from_env(&policy, NULL) == 0, "require env failed");
    CHECK(!policy.require_profile, "COLI_PLACEMENT_REQUIRE_PROFILE=0 ignored");
    unsetenv("COLI_PLACEMENT_REQUIRE_PROFILE");

    CHECK(coli_placement_profile_path() != NULL, "a default profile path is required");
    setenv("COLI_PLACEMENT_PROFILE", "/tmp/somewhere.csv", 1);
    CHECK(strcmp(coli_placement_profile_path(), "/tmp/somewhere.csv") == 0,
          "COLI_PLACEMENT_PROFILE was ignored");
    unsetenv("COLI_PLACEMENT_PROFILE");
}

int main(void) {
    printf("=== placement and shape-set tests ===\n");

    test_shape_set();
    test_tiling();
    test_profile_parse();
    test_profile_rejects_garbage();
    test_profile_roundtrip();
    test_placement_constraints();
    test_row_class_guard();
    test_upload_cost();
    test_permanent_refusal_and_gpu_budget();
    test_placement_forced();
    test_placement_measured();
    test_placement_requires_profile();
    test_placement_structural_shape_sensitivity();
    test_policy_from_env();

    if (g_failures > 0) {
        printf("%d placement check(s) FAILED\n", g_failures);
        return 1;
    }
    printf("placement and shape-set tests passed\n");
    return 0;
}
