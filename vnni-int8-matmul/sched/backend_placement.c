/**
 * backend_placement.c — size-driven engine selection for int8 matmul.
 *
 * See backend_placement.h. Two rules govern this file:
 *
 *   1. Hard constraints are checked before anything is ranked, and a forced
 *      engine that fails one is refused rather than replaced.
 *   2. No timing constant is written here. Ranking comes from the measured
 *      table; when there is none, the fallback is a *structural* argument
 *      (how many times each weight byte is reused) and is labelled as such.
 */

#include "backend_placement.h"

#include "npu_shapes.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Default location of the measured table, relative to vnni-int8-matmul/. */
#define COLI_PLACEMENT_DEFAULT_PROFILE "data/strix_halo_profile.csv"

/*
 * Structural fallback threshold, used only when no measured table is loaded.
 *
 * A dispatch moves `operand_bytes` (activations + weights + output) and
 * performs rows*inner*out MACs. The ratio is how many MACs each moved byte
 * buys. An accelerator that has to allocate, map, cache-flush, submit, wait and
 * read back cannot win when that ratio is ~1, because it moves the same bytes
 * the CPU does and adds a device round trip on top. The default is the smallest
 * prefill row tile: below it, per-dispatch fixed cost is amortised over fewer
 * than 32 rows.
 *
 * This is an argument, not a measurement. A loaded profile overrides it
 * entirely, which is the whole point of measuring.
 */
#define COLI_PLACEMENT_DEFAULT_MIN_REUSE 32.0

const char *coli_placement_source_name(coli_placement_source_t source) {
    switch (source) {
        case COLI_PLACEMENT_SOURCE_MEASURED:   return "measured";
        case COLI_PLACEMENT_SOURCE_ESTIMATED:  return "estimated";
        case COLI_PLACEMENT_SOURCE_STRUCTURAL: return "structural";
        case COLI_PLACEMENT_SOURCE_FORCED:     return "forced";
        default:                               return "none";
    }
}

void coli_placement_caps_init(coli_placement_caps_t *caps) {
    if (!caps) return;
    memset(caps, 0, sizeof(*caps));
}

const char *coli_placement_profile_path(void) {
    const char *path = getenv("COLI_PLACEMENT_PROFILE");
    return (path && *path) ? path : COLI_PLACEMENT_DEFAULT_PROFILE;
}

static bool env_flag(const char *name) {
    const char *value = getenv(name);
    if (!value || !*value) return false;
    return !(strcmp(value, "0") == 0 || strcmp(value, "false") == 0 ||
             strcmp(value, "no") == 0);
}

int coli_placement_policy_from_env(coli_placement_policy_t *policy,
                                   const coli_shape_profile_t *profile) {
    if (!policy) return -EINVAL;
    memset(policy, 0, sizeof(*policy));
    policy->profile = profile;
    policy->forced = COLI_ENGINE_NONE;
    policy->require_profile = env_flag("COLI_PLACEMENT_REQUIRE_PROFILE");

    const char *pinned = getenv("COLI_PLACEMENT");
    if (pinned && *pinned && strcmp(pinned, "auto") != 0 &&
        strcmp(pinned, "AUTO") != 0) {
        coli_engine_t engine = coli_engine_parse(pinned);
        if (engine == COLI_ENGINE_NONE) {
            return -EINVAL;
        }
        policy->forced = engine;
    }
    return 0;
}

static double min_reuse(void) {
    const char *env = getenv("COLI_PLACEMENT_MIN_REUSE");
    if (!env || !*env) return COLI_PLACEMENT_DEFAULT_MIN_REUSE;
    char *end = NULL;
    errno = 0;
    double value = strtod(env, &end);
    if (errno != 0 || !end || end == env || *end != '\0' || value <= 0.0) {
        return COLI_PLACEMENT_DEFAULT_MIN_REUSE;
    }
    return value;
}

/* ── Hard constraints ──
 *
 * Each returns NULL when the engine may run the shape, or the reason it may
 * not. The reason strings are stable so callers can log them verbatim.
 */

static const char *cpu_constraint(const coli_placement_caps_t *caps) {
    if (!caps->cpu_available) return "no AVX-512 VNNI CPU backend";
    return NULL;
}

static const char *gpu_constraint(const coli_placement_caps_t *caps) {
    if (!caps->gpu_available) return "no Vulkan compute context on the iGPU";
    return NULL;
}

static const char *npu_constraint(const coli_placement_caps_t *caps,
                                  int rows, int inner, int out, int fmt) {
    if (!caps->npu_available) return "NPU device or hardware context unavailable";

    if (!caps->npu_kernel_exists ||
        !caps->npu_kernel_exists(rows, inner, out, fmt, caps->npu_kernel_user)) {
        /* AIE-2 is fixed-shape. Widening the match here would dispatch a kernel
         * against a buffer it was not compiled for. */
        return "no exact-shape .npukernel artifact for this shape";
    }

    if (caps->npu_resident_bytes > 0) {
        const size_t needed = coli_npu_operand_bytes(rows, inner, out);
        if (needed == 0) return "operand size overflow";
        if (needed > caps->npu_resident_bytes) {
            return "operand set exceeds NPU resident memory budget";
        }
    }
    return NULL;
}

/* ── Ranking ── */

static double arithmetic_reuse(int rows, int inner, int out) {
    const size_t bytes = coli_npu_operand_bytes(rows, inner, out);
    if (bytes == 0) return 0.0;
    const double macs = (double)rows * (double)inner * (double)out;
    return macs / (double)bytes;
}

static void note_estimate(coli_placement_decision_t *decision,
                          const coli_placement_policy_t *policy,
                          coli_engine_t engine,
                          int rows, int inner, int out, int fmt) {
    decision->estimate_ns[engine] = -1.0;
    decision->match[engine] = COLI_PROFILE_MISS;
    if (!policy->profile) return;

    double ns = -1.0;
    coli_profile_match_t match = coli_profile_estimate_ns(
        policy->profile, engine, rows, inner, out, fmt, &ns);
    decision->match[engine] = match;
    if (match != COLI_PROFILE_MISS) decision->estimate_ns[engine] = ns;
}

coli_engine_t coli_choose_backend(const coli_placement_policy_t *policy,
                                  const coli_placement_caps_t *caps,
                                  int rows, int inner, int out, int fmt,
                                  coli_placement_decision_t *decision) {
    coli_placement_decision_t local;
    if (!decision) decision = &local;
    memset(decision, 0, sizeof(*decision));
    for (int e = 0; e < COLI_ENGINE_COUNT_; ++e) decision->estimate_ns[e] = -1.0;
    decision->engine = COLI_ENGINE_NONE;
    decision->source = COLI_PLACEMENT_SOURCE_NONE;

    if (!policy || !caps) {
        decision->reason = "no placement policy or capability snapshot";
        return COLI_ENGINE_NONE;
    }
    if (rows <= 0 || inner <= 0 || out <= 0) {
        decision->reason = "non-positive matmul shape";
        return COLI_ENGINE_NONE;
    }

    decision->rejected[COLI_ENGINE_CPU] = cpu_constraint(caps);
    decision->rejected[COLI_ENGINE_GPU] = gpu_constraint(caps);
    decision->rejected[COLI_ENGINE_NPU] = npu_constraint(caps, rows, inner, out, fmt);

    for (int e = COLI_ENGINE_CPU; e < COLI_ENGINE_COUNT_; ++e) {
        if (!decision->rejected[e]) {
            note_estimate(decision, policy, (coli_engine_t)e, rows, inner, out, fmt);
        }
    }

    /* A pinned engine is honoured or refused. It is never substituted: an
     * operator who asked for the NPU wants to know the NPU could not run it. */
    if (policy->forced != COLI_ENGINE_NONE) {
        const char *refusal = decision->rejected[policy->forced];
        if (refusal) {
            decision->reason = refusal;
            return COLI_ENGINE_NONE;
        }
        decision->engine = policy->forced;
        decision->source = COLI_PLACEMENT_SOURCE_FORCED;
        decision->reason = "engine pinned by COLI_PLACEMENT";
        return decision->engine;
    }

    /*
     * Measured ranking: cheapest feasible engine wins, but a measurement always
     * outranks an extrapolation. Mixing the two would let a shape scaled from a
     * distant record beat a record taken at exactly this shape — and the
     * extrapolation is the weaker number, especially downwards, where an engine
     * whose fixed cost was never separated out looks arbitrarily cheap.
     * So: if any engine has an exact record, only exact records compete.
     */
    coli_profile_match_t best_match = COLI_PROFILE_MISS;
    for (int e = COLI_ENGINE_CPU; e < COLI_ENGINE_COUNT_; ++e) {
        if (decision->rejected[e]) continue;
        if (decision->match[e] == COLI_PROFILE_EXACT) {
            best_match = COLI_PROFILE_EXACT;
            break;
        }
    }
    const coli_profile_match_t tier =
        (best_match == COLI_PROFILE_EXACT) ? COLI_PROFILE_EXACT
                                           : COLI_PROFILE_ESTIMATE;

    coli_engine_t best = COLI_ENGINE_NONE;
    double best_ns = 0.0;
    for (int e = COLI_ENGINE_CPU; e < COLI_ENGINE_COUNT_; ++e) {
        if (decision->rejected[e]) continue;
        if (decision->match[e] != tier) continue;
        if (best == COLI_ENGINE_NONE || decision->estimate_ns[e] < best_ns) {
            best = (coli_engine_t)e;
            best_ns = decision->estimate_ns[e];
            best_match = decision->match[e];
        }
    }
    if (best != COLI_ENGINE_NONE) {
        decision->engine = best;
        decision->source = (best_match == COLI_PROFILE_EXACT)
            ? COLI_PLACEMENT_SOURCE_MEASURED
            : COLI_PLACEMENT_SOURCE_ESTIMATED;
        decision->reason = (best_match == COLI_PROFILE_EXACT)
            ? "cheapest measured engine for this shape"
            : "cheapest engine estimated from the nearest measured shape";
        return decision->engine;
    }

    /*
     * No usable measurement. Decide anyway, but say that the decision is an
     * argument about data movement rather than an observation — and let the
     * operator refuse it outright.
     */
    if (policy->require_profile) {
        decision->reason =
            "COLI_PLACEMENT_REQUIRE_PROFILE=1 and no measured record covers this "
            "shape; run bench/backend_bench on the Strix Halo machine";
        return COLI_ENGINE_NONE;
    }

    const bool npu_ok = (decision->rejected[COLI_ENGINE_NPU] == NULL);
    const bool gpu_ok = (decision->rejected[COLI_ENGINE_GPU] == NULL);
    const bool cpu_ok = (decision->rejected[COLI_ENGINE_CPU] == NULL);
    const bool amortises = arithmetic_reuse(rows, inner, out) >= min_reuse();

    decision->source = COLI_PLACEMENT_SOURCE_STRUCTURAL;
    if (npu_ok && amortises) {
        decision->engine = COLI_ENGINE_NPU;
        decision->reason = "structural default: weight reuse amortises NPU dispatch cost";
    } else if (gpu_ok && amortises) {
        decision->engine = COLI_ENGINE_GPU;
        decision->reason = "structural default: weight reuse amortises iGPU dispatch cost";
    } else if (cpu_ok) {
        decision->engine = COLI_ENGINE_CPU;
        decision->reason = "structural default: too little weight reuse to pay for a device round trip";
    } else if (gpu_ok) {
        decision->engine = COLI_ENGINE_GPU;
        decision->reason = "structural default: only the iGPU is available";
    } else if (npu_ok) {
        decision->engine = COLI_ENGINE_NPU;
        decision->reason = "structural default: only the NPU is available";
    } else {
        decision->source = COLI_PLACEMENT_SOURCE_NONE;
        decision->reason = "every engine refused this shape";
    }
    return decision->engine;
}
