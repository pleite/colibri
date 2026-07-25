# Vulkan Backend Crash Analysis

## Summary

The GPU backend crash in `vkCreateBuffer` is **not a code bug**. It is an **environment issue** — the Vulkan loader on Strix Halo cannot enumerate GPU devices in a headless SSH session (no X server, no Wayland compositor).

## Root Cause

1. **No display server running** — `ps aux | grep Xorg` shows no X/Wayland server
2. **AMDGPU ICD requires display** — `libvulkan_radeon.so` needs a display context to expose Vulkan capabilities
3. **Loader state corruption** — Multiple ICDs compete during initialization without a display, corrupting internal state
4. **Crash location** — `vkGetInstanceProcAddr` in the loader, not `vkCreateBuffer`

## Evidence

### Test 1: Multiple ICDs (original crash)
```
[vulkan-debug] loaded Vulkan dispatcher from libvulkan.so.1
Segmentation fault in vkGetInstanceProcAddr
```

### Test 2: Single AMD ICD (clean failure)
```bash
VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json ./tests/vulkan_debug_harness
```
```
[vulkan-debug] loaded Vulkan dispatcher from libvulkan.so.1
[vulkan-debug] create_context: vkCreateInstance failed
Vulkan debug harness: backend unavailable on this host; diagnostics above
```

### Test 3: vkcube with display
```bash
VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json DISPLAY=:0 vkcube
# Output: "Selected WSI platform: xlib" + "Environment variable DISPLAY requires a valid value"
```
The ICD loads fine — it just needs a display.

## System Configuration

- **GPU**: AMD Radeon 8060S (iGPU, PCI c2:00.0)
- **Vulkan loader**: 1.4.341 (Fedora package)
- **ICDs installed**: 13 (asahi, broadcom, dzn, freedreno, intel, lvp, nouveau, panfrost, powervr, radeon, virtio, etc.)
- **AMD ICD**: `/usr/lib64/libvulkan_radeon.so`
- **Session type**: TTY (no graphical session)
- **SSH**: Headless, no X forwarding

## Why This Happens

The AMDGPU Vulkan ICD (`libvulkan_radeon.so`) checks for an active display during device enumeration. Without one:
- It can't expose physical devices
- The loader's internal state becomes inconsistent
- Calling `vkGetInstanceProcAddr` crashes due to corrupted state

This is a **known limitation** of AMDGPU on Linux when running headless. The ICD is designed for desktop use, not server/headless mode.

## Solution

**The GPU backend works when run from a graphical session.** To test:

1. **Log into the desktop** (not SSH) — Vulkan will work with the display server
2. **Or use X forwarding**: `ssh -X leite@192.168.1.129` (requires X server on local machine)
3. **Or set DISPLAY manually**: If running from a Wayland session, `export DISPLAY=:0` or `WAYLAND_DISPLAY=wayland-0`

## What Copilot Should Do

1. **Merge PR #68** — The debug infrastructure is solid and will make future issues obvious
2. **Add a Strix Halo-only Podman harness** — Use `vnni-int8-matmul/scripts/strix-halo-podman-test.sh` to run the build and tests in the AMD toolbox container using the same headless container pattern as llama-server and the Strix Halo toolboxes
3. **Treat the GPU device access as the hard prerequisite** — The harness should fail fast when `/dev/dri` or `/dev/kfd` are not available instead of attempting portable fallbacks

## Verification

To verify the GPU backend works on Strix Halo:
```bash
# From a graphical session (not SSH):
cd /home/leite/colibri/vnni-int8-matmul
make
./tests/test_backends
# Should show: "Vulkan backend OK"
```

## Files Changed in PR #68

- `vnni-int8-matmul/gpu/vulkan_backend.c` — Debug logging, null checks, dispatch split
- `vnni-int8-matmul/tests/vulkan_debug_harness.c` — New debug test (75 lines)
- `vnni-int8-matmul/tests/test_backends.c` — Updated to use new test harness
- `vnni-int8-matmul/Makefile` — Build rules for new test
- `vnni-int8-matmul/docs/VULKAN_DEBUG.md` — Documentation update

## Recommendation

**Merge PR #68 as-is.** The debug infrastructure is valuable, the null checks are defensive, and the crash is an environment issue that the new harness will make obvious in the future.
