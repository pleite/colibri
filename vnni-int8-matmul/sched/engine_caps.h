#ifndef COLI_ENGINE_CAPS_H
#define COLI_ENGINE_CAPS_H

/**
 * engine_caps.h — fill a placement capability snapshot from the real devices.
 *
 * `coli_placement_caps_t` is deliberately plain data so that the scheduler and
 * its tests build on a host with none of the silicon. Something still has to
 * ask the hardware, though, and until now nothing did: `npu_resident_bytes` was
 * consumed by `backend_placement.c` and computed by no one, so the residency
 * constraint never fired outside a test.
 *
 * This translation unit is that missing half, and it is the *only* part of
 * sched/ that links the backends. It is not in SCHED_OBJS: a target that wants
 * it links `sched/engine_caps.o` together with the CPU, GPU and NPU backends.
 */

#include "backend_placement.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Fill `caps` by probing the machine:
 *
 *   cpu_available        AVX-512 VNNI (strix_cpu_is_supported)
 *   gpu_available        a headless Vulkan compute context on the iGPU
 *   gpu_resident_bytes   largest DEVICE_LOCAL heap
 *   npu_available        an open XDNA 2 hardware context
 *   npu_resident_bytes   the mapped amdxdna device heap
 *   npu_kernel_exists    the NPU backend's exact-shape artifact lookup
 *
 * Probing opens the devices, which is why this is an explicit call and not
 * something `coli_placement_caps_init()` does. `weights_resident` is left
 * false: residency of a particular tensor is the caller's knowledge, not the
 * device's.
 */
void coli_engine_caps_probe(coli_placement_caps_t *caps);

#ifdef __cplusplus
}
#endif

#endif /* COLI_ENGINE_CAPS_H */
