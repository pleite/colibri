#ifndef XDNA2_XRT_DRIVER_H
#define XDNA2_XRT_DRIVER_H

/**
 * xdna2_xrt_driver.h — XRT / XDNA shim control plane for the XDNA 2 NPU.
 *
 * Strix Halo exclusive, like the rest of c/npu_kernels. This file adds *no*
 * execution path and *no* fallback: it only answers the question "is this host
 * running the official AMD stack (XRT plus libxrt_driver_xdna, the XDNA shim)
 * and does the NPU open through it?".
 *
 * Why only that
 * -------------
 * The shim teaches XRT how to drive /dev/accel/accel0 (see
 * docs/xdna_shim_guide.md). Dispatch through XRT needs `.xclbin` artifacts and
 * a kernel argument convention produced by the AMD AIE toolchain; this
 * repository only has the self-describing `.npukernel` container consumed by
 * the DRM path. Writing an XRT submit/wait path against artifacts that do not
 * exist would mean guessing an ABI, which the NPU guardrails forbid. So the
 * dataflow stays on DRM ioctls and this layer is limited to what can be
 * verified on hardware today: presence and device open.
 *
 * The XRT code is compiled only when the build found XRT and defined
 * COLI_NPU_XRT_AVAILABLE. No XRT type, handle or entry point is re-declared
 * here; the official <xrt/xrt_device.h> is included or nothing is.
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Control plane selection ──
 *
 * XDNA2_DRIVER selects how the runtime validates the device before it starts
 * issuing DRM ioctls:
 *
 *   auto (default) — probe XRT when it is compiled in, report what was found,
 *                    and continue either way.
 *   drm            — skip XRT entirely.
 *   xrt            — require the XRT + shim stack; initialisation fails with
 *                    -ENOSYS when it is missing instead of quietly degrading.
 */
typedef enum {
    XDNA2_CONTROL_PLANE_AUTO = 0,
    XDNA2_CONTROL_PLANE_DRM  = 1,
    XDNA2_CONTROL_PLANE_XRT  = 2,
} xdna2_control_plane_t;

/** "auto", "drm" or "xrt"; "unknown" for an out-of-range value. */
const char *xdna2_control_plane_name(xdna2_control_plane_t plane);

/**
 * Parse a control-plane name (case-insensitive, NULL or empty means auto).
 * Returns 0 and stores the value, or -EINVAL for an unknown name.
 */
int xdna2_control_plane_parse(const char *value, xdna2_control_plane_t *out);

/** Parse the XDNA2_DRIVER environment variable. Same contract as above. */
int xdna2_control_plane_from_env(xdna2_control_plane_t *out);

/**
 * Resolve a requested control plane against this host.
 *
 * AUTO resolves to XRT when the probe succeeds and to DRM otherwise. DRM always
 * resolves to DRM. XRT resolves to XRT or fails with -ENOSYS. The resolved
 * value is never AUTO.
 */
int xdna2_control_plane_resolve(xdna2_control_plane_t requested,
                                xdna2_control_plane_t *resolved);

/* ── XRT probe ── */

/** True when this translation unit was built against real XRT headers. */
bool xdna2_xrt_compiled_in(void);

/**
 * Open and immediately close the NPU through XRT (device index from
 * XDNA2_XRT_DEVICE_INDEX, default 0).
 *
 * Returns 0 when the official stack drives the device, -ENOSYS when XRT was not
 * compiled in, or -ENODEV when the open failed. The result is cached; the first
 * call performs the probe.
 */
int xdna2_xrt_probe(void);

/** Human-readable outcome of the probe. Never NULL. */
const char *xdna2_xrt_status(void);

#ifdef __cplusplus
}
#endif

#endif /* XDNA2_XRT_DRIVER_H */
