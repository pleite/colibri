/**
 * xdna2_xrt_driver.c — XRT / XDNA shim control plane for the XDNA 2 NPU.
 *
 * See xdna2_xrt_driver.h for scope. There is no execution path and no fallback
 * here: when XRT is absent the probe says so and the caller decides.
 */

#include "xdna2_xrt_driver.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef COLI_NPU_XRT_AVAILABLE
/*
 * Official XRT C API. Never re-declare xrtDeviceHandle or the entry points:
 * either the header is present and this path is built, or the build defines
 * nothing and the stub below reports -ENOSYS.
 */
#include <xrt/xrt_device.h>
#endif

/* ── Control plane selection ─────────────────────────────────────────────── */

const char *xdna2_control_plane_name(xdna2_control_plane_t plane) {
    switch (plane) {
        case XDNA2_CONTROL_PLANE_AUTO: return "auto";
        case XDNA2_CONTROL_PLANE_DRM:  return "drm";
        case XDNA2_CONTROL_PLANE_XRT:  return "xrt";
        default:                       return "unknown";
    }
}

int xdna2_control_plane_parse(const char *value, xdna2_control_plane_t *out) {
    if (!out) return -EINVAL;

    if (!value) {
        *out = XDNA2_CONTROL_PLANE_AUTO;
        return 0;
    }

    /* Trim surrounding blanks so `XDNA2_DRIVER=" xrt"` behaves. */
    while (*value == ' ' || *value == '\t') value++;
    size_t len = strlen(value);
    while (len > 0 && (value[len - 1] == ' ' || value[len - 1] == '\t')) len--;

    if (len == 0) {
        *out = XDNA2_CONTROL_PLANE_AUTO;
        return 0;
    }

    char normalized[8];
    if (len >= sizeof(normalized)) return -EINVAL;
    for (size_t i = 0; i < len; ++i) {
        int c = (int)(unsigned char)value[i];
        normalized[i] = (char)((c >= 'A' && c <= 'Z') ? c + 32 : c);
    }
    normalized[len] = '\0';

    if (strcmp(normalized, "auto") == 0) {
        *out = XDNA2_CONTROL_PLANE_AUTO;
        return 0;
    }
    if (strcmp(normalized, "drm") == 0) {
        *out = XDNA2_CONTROL_PLANE_DRM;
        return 0;
    }
    if (strcmp(normalized, "xrt") == 0) {
        *out = XDNA2_CONTROL_PLANE_XRT;
        return 0;
    }
    return -EINVAL;
}

int xdna2_control_plane_from_env(xdna2_control_plane_t *out) {
    const char *env = getenv("XDNA2_DRIVER");
    int ret = xdna2_control_plane_parse(env, out);
    if (ret < 0) {
        fprintf(stderr,
                "xdna2: XDNA2_DRIVER='%s' is not a control plane "
                "(expected auto, drm or xrt)\n", env ? env : "");
    }
    return ret;
}

int xdna2_control_plane_resolve(xdna2_control_plane_t requested,
                                xdna2_control_plane_t *resolved) {
    if (!resolved) return -EINVAL;

    switch (requested) {
        case XDNA2_CONTROL_PLANE_DRM:
            *resolved = XDNA2_CONTROL_PLANE_DRM;
            return 0;

        case XDNA2_CONTROL_PLANE_XRT:
            if (xdna2_xrt_probe() < 0) {
                fprintf(stderr,
                        "xdna2: XDNA2_DRIVER=xrt but the XRT/XDNA shim stack is "
                        "unusable: %s\n", xdna2_xrt_status());
                return -ENOSYS;
            }
            *resolved = XDNA2_CONTROL_PLANE_XRT;
            return 0;

        case XDNA2_CONTROL_PLANE_AUTO:
            *resolved = (xdna2_xrt_probe() == 0) ? XDNA2_CONTROL_PLANE_XRT
                                                 : XDNA2_CONTROL_PLANE_DRM;
            return 0;

        default:
            return -EINVAL;
    }
}

/* ── XRT probe ───────────────────────────────────────────────────────────── */

static int g_probe_done = 0;
static int g_probe_result = -ENOSYS;
static const char *g_probe_status = "not probed";

bool xdna2_xrt_compiled_in(void) {
#ifdef COLI_NPU_XRT_AVAILABLE
    return true;
#else
    return false;
#endif
}

#ifdef COLI_NPU_XRT_AVAILABLE
static unsigned int xrt_device_index(void) {
    const char *env = getenv("XDNA2_XRT_DEVICE_INDEX");
    if (!env || !*env) return 0u;
    char *end = NULL;
    errno = 0;
    unsigned long value = strtoul(env, &end, 10);
    if (errno != 0 || !end || *end != '\0' || value > 0xFFFFFFFFul) {
        fprintf(stderr,
                "xdna2: XDNA2_XRT_DEVICE_INDEX='%s' is not a device index; "
                "using 0\n", env);
        return 0u;
    }
    return (unsigned int)value;
}
#endif

int xdna2_xrt_probe(void) {
    if (g_probe_done) return g_probe_result;
    g_probe_done = 1;

#ifdef COLI_NPU_XRT_AVAILABLE
    xrtDeviceHandle device = xrtDeviceOpen(xrt_device_index());
    if (!device) {
        g_probe_result = -ENODEV;
        g_probe_status = "XRT is installed but no NPU device opened through it "
                         "(is libxrt_driver_xdna, the XDNA shim, installed?)";
        return g_probe_result;
    }
    xrtDeviceClose(device);
    g_probe_result = 0;
    g_probe_status = "XRT opened the NPU through the XDNA shim";
#else
    g_probe_result = -ENOSYS;
    g_probe_status = "built without XRT; DRM ioctl control plane only";
#endif

    return g_probe_result;
}

const char *xdna2_xrt_status(void) {
    if (!g_probe_done) (void)xdna2_xrt_probe();
    return g_probe_status;
}
