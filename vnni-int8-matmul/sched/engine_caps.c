/**
 * engine_caps.c — the hardware half of the placement capability snapshot.
 *
 * See engine_caps.h. Every value here comes from a device query. Nothing is
 * defaulted to a plausible number: an unavailable engine reports 0 bytes, which
 * `backend_placement.c` reads as "unknown, do not use residency as a
 * constraint" rather than as "no room".
 */

#include "engine_caps.h"

#include "cpu/vnni_cpu_backend.h"
#include "gpu/vulkan_backend.h"
#include "npu/xdna2_backend.h"

static bool npu_kernel_exists_cb(int rows, int inner, int out, int fmt,
                                 void *user) {
    (void)user;
    return strix_xdna2_kernel_exists(rows, inner, out, fmt) == 1;
}

void coli_engine_caps_probe(coli_placement_caps_t *caps) {
    if (!caps) return;
    coli_placement_caps_init(caps);

    caps->cpu_available = (strix_cpu_is_supported() == 1);

    caps->gpu_available = (strix_vulkan_is_supported() == 1);
    if (caps->gpu_available) {
        caps->gpu_resident_bytes = strix_vulkan_resident_bytes();
    }

    caps->npu_available = (strix_xdna2_is_supported() == 1);
    if (caps->npu_available) {
        caps->npu_resident_bytes = strix_xdna2_resident_bytes();
        caps->npu_kernel_exists = npu_kernel_exists_cb;
    }
}
