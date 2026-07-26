/**
 * placement_report.c — print the placement decision for every enumerated shape,
 * on this machine, with its source and every refusal.
 *
 * "The NPU is being used" is an assertion until something prints which engine
 * ran what and why the others did not. This is that something, for the static
 * half of the question: the capability snapshot comes from the real devices
 * (sched/engine_caps.h), the shapes come from the single enumeration in
 * sched/npu_shapes.c, and the decision comes from the one placement function.
 *
 * It dispatches nothing, so it is safe to run anywhere: on a host without the
 * silicon every engine is refused and the report says exactly that.
 *
 * Usage:
 *   tools/placement_report [--profile PATH] [--warm]
 *
 *   --profile PATH   measured table to rank with (default:
 *                    COLI_PLACEMENT_PROFILE, else data/strix_halo_profile.csv)
 *   --warm           report the decision for weights already resident on every
 *                    engine, which is the steady state for a hot expert
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sched/backend_placement.h"
#include "sched/engine_caps.h"
#include "sched/npu_shapes.h"
#include "sched/shape_profile.h"

static void print_caps(const coli_placement_caps_t *caps) {
    printf("# host capabilities\n");
    printf("#   cpu available            %s\n", caps->cpu_available ? "yes" : "no");
    printf("#   gpu available            %s\n", caps->gpu_available ? "yes" : "no");
    printf("#   gpu resident bytes       %zu%s\n", caps->gpu_resident_bytes,
           caps->gpu_resident_bytes ? "" : " (unknown; not used as a constraint)");
    printf("#   npu available            %s\n", caps->npu_available ? "yes" : "no");
    printf("#   npu resident bytes       %zu%s\n", caps->npu_resident_bytes,
           caps->npu_resident_bytes ? "" : " (unknown; not used as a constraint)");
}

int main(int argc, char **argv) {
    const char *profile_path = NULL;
    bool warm = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--warm") == 0) {
            warm = true;
        } else if (strcmp(argv[i], "--profile") == 0 && i + 1 < argc) {
            profile_path = argv[++i];
        } else {
            fprintf(stderr, "usage: %s [--profile PATH] [--warm]\n", argv[0]);
            return 2;
        }
    }
    if (!profile_path) profile_path = coli_placement_profile_path();

    coli_shape_profile_t *profile = coli_profile_create();
    if (!profile) {
        fprintf(stderr, "placement_report: out of memory\n");
        return 1;
    }
    const int loaded = coli_profile_load_file(profile, profile_path);
    if (loaded == 0) {
        printf("# profile: %s (%zu record(s))\n", profile_path,
               coli_profile_count(profile));
    } else if (loaded == -ENOENT) {
        printf("# profile: %s is absent; every decision below is structural\n",
               profile_path);
        coli_profile_free(profile);
        profile = NULL;
    } else {
        fprintf(stderr, "placement_report: '%s' did not parse (%d); refusing to "
                        "rank from a half-read table\n", profile_path, loaded);
        coli_profile_free(profile);
        return 1;
    }

    coli_placement_caps_t caps;
    coli_engine_caps_probe(&caps);
    if (warm) {
        for (int e = COLI_ENGINE_CPU; e < COLI_ENGINE_COUNT_; ++e) {
            caps.weights_resident[e] = true;
        }
        printf("# residency: weights assumed already uploaded on every engine\n");
    }
    print_caps(&caps);

    coli_placement_policy_t policy;
    if (coli_placement_policy_from_env(&policy, profile) != 0) {
        fprintf(stderr, "placement_report: COLI_PLACEMENT names no engine\n");
        coli_profile_free(profile);
        return 1;
    }

    printf("rows,inner,out,engine,source,reason,cpu_rejected,gpu_rejected,"
           "npu_rejected,npu_permanent\n");

    size_t count = 0;
    const coli_npu_shape_t *shapes = coli_npu_shape_set(&count);
    for (size_t i = 0; i < count; ++i) {
        coli_placement_decision_t d;
        const coli_engine_t engine = coli_choose_backend(
            &policy, &caps, shapes[i].rows, shapes[i].inner, shapes[i].out,
            1 /* XDNA2_FMT_INT8 */, &d);

        printf("%d,%d,%d,%s,%s,\"%s\",\"%s\",\"%s\",\"%s\",%s\n",
               shapes[i].rows, shapes[i].inner, shapes[i].out,
               coli_engine_name(engine),
               coli_placement_source_name(d.source),
               d.reason ? d.reason : "",
               d.rejected[COLI_ENGINE_CPU] ? d.rejected[COLI_ENGINE_CPU] : "",
               d.rejected[COLI_ENGINE_GPU] ? d.rejected[COLI_ENGINE_GPU] : "",
               d.rejected[COLI_ENGINE_NPU] ? d.rejected[COLI_ENGINE_NPU] : "",
               d.rejected_permanent[COLI_ENGINE_NPU] ? "yes" : "no");
    }

    coli_profile_free(profile);
    return 0;
}
