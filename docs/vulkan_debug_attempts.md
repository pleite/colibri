# Vulkan on Strix Halo — Validated Root Cause and Guardrails

**Status:** resolved. Every assumption in the earlier revision of this document
has been tested; four of the five were wrong. This document records what was
actually true, how it was proved, and what must not be attempted again.

**Scope:** AMD Ryzen AI Max+ 395 "Strix Halo", RADV GFX1151 iGPU, headless,
inside Podman. Nothing here is portable and nothing here has a fallback path.

---

## 1. The original symptom

`vnni-int8-matmul` reported `strix_vulkan_backend_name() == "vulkan-unavailable"`
and, when it did reach the driver, segfaulted at or near address `0x0`. The
crash appeared to happen while resolving `vkGetDeviceProcAddr` through the
function pointer returned by `dlsym(handle, "vkGetInstanceProcAddr")`, so the
investigation focused on `dlsym`, `RTLD_*` flags, and the Vulkan loader.

That focus was misplaced.

---

## 2. Root cause

`vnni-int8-matmul/vulkan/vulkan.h` was a **hand-written re-declaration of the
Vulkan ABI**. Several of its structures did not match the real layout, so the
driver wrote past the end of the caller's stack objects.

The decisive case is `VkPhysicalDeviceProperties`:

| Type | Real size (`vulkan_core.h`) | Shim size | Consequence |
|---|---|---|---|
| `VkPhysicalDeviceProperties` | **824 bytes** | **4 bytes** | `vkGetPhysicalDeviceProperties()` overruns the caller's stack by **820 bytes** |
| `VkPhysicalDeviceMemoryProperties` | **520 bytes** | **132 bytes** | `vkGetPhysicalDeviceMemoryProperties()` overruns by **388 bytes** |

`create_context()` called `vkGetPhysicalDeviceProperties(devices[i], &props)`
with `props` declared as the 4-byte shim type. In the same stack frame lived the
`VkPhysicalDevice devices[8]` array, the `VkQueueFamilyProperties families[8]`
array, the `dispatch` pointer and the saved return address. The driver wiped all
of them.

That explains every confusing observation at once:

* the crash address was garbage, so it looked like a null dereference;
* the function pointer really was non-NULL when checked, because it was
  corrupted *after* the check;
* the same source worked in `vulkan_debug_harness` and failed in the test
  harness, because the two binaries have different stack layouts and therefore
  overwrite different things;
* changing `RTLD_LOCAL`/`RTLD_NOW`/`RTLD_GLOBAL` changed nothing, because the
  loader was never the problem.

### How this was proved

The two headers were compiled side by side and their layouts printed:

```c
printf("%zu\n", sizeof(VkPhysicalDeviceProperties));
printf("%zu %zu %zu\n", offsetof(VkMemoryRequirements, size),
                        offsetof(VkMemoryRequirements, alignment),
                        offsetof(VkMemoryRequirements, memoryTypeBits));
```

| Fact | Official `<vulkan/vulkan_core.h>` | Old shim |
|---|---|---|
| `sizeof(VkPhysicalDeviceProperties)` | 824 | 4 |
| `sizeof(VkPhysicalDeviceMemoryProperties)` | 520 | 132 |
| `sizeof(VkMemoryType)` (array stride) | 8 | 4 |
| `VkMemoryRequirements` field order | `size`(0), `alignment`(8), `memoryTypeBits`(16) | `memoryTypeBits`(0), `size`(8), `alignment`(16) |
| `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` | 7 | 0 (`SAMPLER`) |
| `VK_PIPELINE_BIND_POINT_COMPUTE` | 1 | 0 (`GRAPHICS`) |
| `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT` | 0x20 | 0x04 (`UNIFORM_TEXEL_BUFFER`) |
| `VK_ERROR_INITIALIZATION_FAILED` | −3 | 1 |
| `VK_STRUCTURE_TYPE_*` | 15 of 19 values differ | invented sequential values |

Even with the stack corruption removed, the wrong enum values alone would have
made every descriptor, buffer and pipeline binding invalid.

---

## 3. Assumption-by-assumption verdict

| # | Original assumption | Verdict | Evidence |
|---|---|---|---|
| 1 | "The crash is in `vkGetInstanceProcAddr`" | **Refuted** | `vkGetInstanceProcAddr` is correctly typed in the shim and behaves per spec. The corruption happened earlier, in `vkGetPhysicalDeviceProperties`. |
| 2 | "The issue is with `dlsym`" | **Refuted** | `dlsym(handle, "vkGetInstanceProcAddr")` is the one lookup the Vulkan Loader Interface explicitly sanctions. It always returned the correct pointer. |
| 3 | "The instance is valid" | **Correct, but irrelevant** | `vkCreateInstance` succeeded. The instance handle was later clobbered by the stack overrun, which is a different failure. |
| 4 | "The issue is with the test harness" | **Refuted** | The bug is in the backend. The harness only changed the stack layout, which changed *which* bytes got destroyed and therefore whether the crash was visible. |
| 5 | "The crash is a null pointer dereference" | **Refuted** | It is a stack buffer overflow. The apparent null pointer is corrupted memory. |

One further observation from the original report was itself a separate,
unrelated bug: `strix_vulkan_backend_name()` returned `"vulkan-unavailable"`
because it read a global that was only set as a side effect of a previous
`strix_vulkan_matmul()` call. It never called `load_dispatch()` at all, so it
could not have crashed there. It now performs lazy initialisation.

---

## 4. The fix

| Change | File |
|---|---|
| Deleted the hand-rolled ABI header | `vnni-int8-matmul/vulkan/vulkan.h` (removed) |
| Dispatch tables built only from `PFN_vk*` typedefs in the official header | `gpu/vulkan_dispatch.h` |
| `dlsym` used for `vkGetInstanceProcAddr` only; everything else resolved through `vkGetInstanceProcAddr` → `vkGetDeviceProcAddr` | `gpu/vulkan_backend.c` |
| Strix Halo exclusive device selection (AMD vendor ID, `INTEGRATED_GPU`, GFX1151/8060S/8050S) | `gpu/vulkan_backend.c` |
| Headless: zero instance extensions requested | `gpu/vulkan_backend.c` |
| Process-wide cached context instead of a `VkDevice` per matmul | `gpu/vulkan_backend.c` |
| Lazy init in `strix_vulkan_backend_name()`, plus `strix_vulkan_failure_reason()` | `gpu/vulkan_backend.{c,h}` |
| GLSL source committed so `gpu/comp.spv` is reproducible | `gpu/comp.comp` |

### Loader contract now honoured

Per the Khronos *Vulkan Loader Interface* specification, an application may take
exactly one symbol from `libvulkan.so.1` with `dlsym`: `vkGetInstanceProcAddr`.
Global commands (`vkCreateInstance`, `vkEnumerateInstanceVersion`) are resolved
by calling it with a `NULL` instance; instance commands with a live
`VkInstance`; device commands through `vkGetDeviceProcAddr`. Taking instance or
device commands directly from `dlsym` bypasses the loader trampolines and is
only accidentally correct when a single ICD is installed.

### Headless is the normal case

The backend requests **no** instance extensions. `VK_KHR_surface`,
`VK_KHR_xlib_surface` and friends are what create a dependency on a display
server; a compute-only pipeline needs none of them. Xvfb, `DISPLAY`,
`WAYLAND_DISPLAY` and `XAUTHORITY` are irrelevant and are no longer passed into
the container by `scripts/strix-halo-podman-test.sh`.

---

## 5. Guardrails — do not do these

These are the approaches that caused, hid or prolonged the bug. They must not be
reintroduced.

1. **Never hand-write Vulkan types.** No `typedef struct Vk...`, no
   `#define VK_STRUCTURE_TYPE_...`, no `PFN_` typedef written by hand. Include
   `<vulkan/vulkan_core.h>`. If the header is missing, fail the build; do not
   substitute a local declaration.
2. **Never hand-copy kernel UAPI structs.** The same class of bug exists in
   `c/npu_kernels/`: use `struct amdxdna_drm_*` from `<drm/amdxdna_accel.h>` and
   named `enum amdxdna_drm_get_param` values, never a local copy and never a
   magic parameter number.
3. **Never pass an undersized output struct to a driver.** Any `vkGet*` or
   `ioctl` that fills a structure must be given a full-size, zero-initialised
   object of the official type.
4. **Do not chase `dlopen`/`dlsym` flags.** `RTLD_NOW | RTLD_LOCAL` is correct.
   Toggling `RTLD_GLOBAL`, dropping `RTLD_NOW`, or linking `-lvulkan` directly
   does not fix ABI corruption; it only moves it.
5. **Do not add a display server.** Xvfb, X11 socket mounts and `DISPLAY`
   plumbing are not a fix for a headless compute backend.
6. **Do not add a fallback.** No lavapipe/llvmpipe, no discrete GPU, no CPU
   path behind the Vulkan or NPU APIs. A silent fallback turns a hardware bug
   into a performance mystery. Fail and report the reason instead.
7. **Do not relax the device filter.** `VNNI_STRIX_DEVICE_NAME` may only add an
   accepted device-name substring; the AMD vendor ID and `INTEGRATED_GPU` checks
   are unconditional and must stay that way.
8. **Do not debug memory corruption with printf alone.** Build with
   `-fsanitize=address` first. ASan reports a stack-buffer-overflow in
   `vkGetPhysicalDeviceProperties` immediately; a week of `dlsym` printf
   debugging did not.

---

## 6. Verifying a change

On any host (the backend must degrade to a clean, explained skip):

```bash
cd vnni-int8-matmul
make
make test
```

Expected on a non-Strix-Halo host — no crash, an explicit reason:

```
Vulkan backend SKIP (vkCreateInstance failed)
XDNA2 backend SKIP (cannot open /dev/accel/accel0 ...)
```

On Strix Halo, headless, in Podman:

```bash
cd vnni-int8-matmul
./scripts/strix-halo-podman-test.sh
```

With diagnostics:

```bash
VNNI_VULKAN_DEBUG=1 ./tests/vulkan_debug_harness
```

which prints every enumerated physical device with its vendor ID, device ID and
type, the accept/reject decision for each, and the selected device name.

### Regression guard

If a Vulkan-related crash reappears, check these in order before anything else:

```bash
# 1. Is any Vulkan type declared outside the official header?
grep -rn "typedef struct Vk\|#define VK_STRUCTURE_TYPE" vnni-int8-matmul/ c/

# 2. Does the backend still resolve only vkGetInstanceProcAddr via dlsym?
grep -n "dlsym" vnni-int8-matmul/gpu/vulkan_backend.c

# 3. Reproduce under ASan.
cd vnni-int8-matmul
make clean
make CFLAGS="-O1 -g -fsanitize=address -std=c11" LDFLAGS="-lm -ldl -fsanitize=address"
./tests/vulkan_debug_harness
```

---

## 7. References

* Khronos, *Vulkan Loader Interface* — application-to-loader interface, rules
  for `vkGetInstanceProcAddr` and `vkGetDeviceProcAddr`.
* Khronos, *Vulkan Specification*, §"Devices and Queues" —
  `vkGetPhysicalDeviceProperties`, `VkPhysicalDeviceProperties`.
* Khronos, *Vulkan Specification*, §"Memory Allocation" —
  `VkPhysicalDeviceMemoryProperties`, `VkMemoryRequirements`,
  `VkMemoryPropertyFlagBits`.
* Khronos, *Vulkan Specification*, §"Synchronization and Cache Control" — host
  visibility of shader writes, `VK_ACCESS_HOST_READ_BIT`.
* Mesa RADV, `VP_VULKANINFO_AMD_Radeon_8060S_Graphics_(RADV_GFX1151)_25_3_6.json`
  in `vnni-int8-matmul/` — the captured device profile for this machine.
* `docs/strix-halo-npu.md` — the XDNA 2 NPU counterpart to this document.
