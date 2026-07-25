# Vulkan Debug Attempts — Context for Copilot

## Problem Statement

The Vulkan backend in `c/vulkan_backend.c` crashes with a segfault when trying to create a Vulkan context. The crash occurs in `vkGetInstanceProcAddr` when trying to resolve `vkGetDeviceProcAddr`.

## What We Know

### Working Setup
- Vulkan works on the host with Xvfb (virtual display)
- `vulkaninfo` returns successfully
- `vulkan_debug_harness` works when run directly (not through test harness)
- The basic Vulkan setup (instance, physical device, queue family) works

### Failing Setup
- `strix_vulkan_backend_name()` returns "vulkan-unavailable"
- `g_vulkan_available` is false
- `load_dispatch()` crashes when trying to get `vkGetInstanceProcAddr` via `dlsym`

### Key Observations

1. **The crash is in `dlsym`, not in Vulkan itself**
   - We use `dlopen` to load `libvulkan.so.1`
   - We use `dlsym` to get `vkGetInstanceProcAddr`
   - The crash happens when we call `get_proc_addr(instance, "vkGetDeviceProcAddr")`
   - The function pointer from `dlsym` is valid (not NULL) but the call crashes

2. **The same code works when run directly**
   - `vulkan_debug_harness` works fine
   - It uses the same `load_dispatch` function
   - The difference is that `vulkan_debug_harness` is a standalone test, not the test harness that loads multiple backends

3. **The issue is specific to the test harness**
   - The test harness loads multiple backends
   - It calls `strix_vulkan_backend_name()` which calls `load_dispatch()`
   - The crash happens in `load_dispatch()`

4. **We tried many approaches and they all failed**
   - Changed `RTLD_LOCAL` to `RTLD_GLOBAL`
   - Removed `RTLD_NOW` flag
   - Used global `vkGetInstanceProcAddr` instead of `dlsym`
   - Added null checks
   - Changed function pointer types
   - Used GDB to get stack traces
   - All approaches failed with the same crash

## Assumptions That May Be Wrong

### Assumption 1: The crash is in `vkGetInstanceProcAddr`
- **Reality**: The crash is in the function we get from `dlsym`, which we assume is `vkGetInstanceProcAddr`
- **But**: We never verified that the function we got from `dlsym` is actually `vkGetInstanceProcAddr`
- **Test**: We should print the function address and compare it to the global `vkGetInstanceProcAddr`

### Assumption 2: The issue is with `dlsym`
- **Reality**: The issue might be with how we're using the function pointer, not with `dlsym` itself
- **But**: We tried using the global `vkGetInstanceProcAddr` and it also failed
- **Test**: We should try calling `vkGetInstanceProcAddr` directly (not via `dlsym`) in the test harness

### Assumption 3: The instance is valid
- **Reality**: We assume the instance is valid when we call `get_proc_addr(instance, "vkGetDeviceProcAddr")`
- **But**: The instance might be corrupted or invalid
- **Test**: We should check if the instance is valid before calling `get_proc_addr`

### Assumption 4: The issue is with the test harness
- **Reality**: The issue might be with the Vulkan backend code itself, not the test harness
- **But**: The same code works in `vulkan_debug_harness`
- **Test**: We should try running `vulkan_debug_harness` through the test harness to see if it crashes

### Assumption 5: The crash is a null pointer dereference
- **Reality**: The crash is at address `0x0` which suggests a null pointer dereference
- **But**: The function pointer is not NULL (we checked)
- **Test**: We should check if the instance is NULL or invalid

## What We Should Try Next

### 1. Verify the function pointer from `dlsym`
- Print the address of the function we get from `dlsym`
- Compare it to the global `vkGetInstanceProcAddr`
- If they're different, we know we're not getting the right function

### 2. Try using the global `vkGetInstanceProcAddr` directly
- Don't use `dlsym` at all
- Just call `vkGetInstanceProcAddr` directly
- This should work if the Vulkan loader is properly linked

### 3. Check if the instance is valid
- Before calling `get_proc_addr`, check if the instance is valid
- Try calling other instance functions to see if they work

### 4. Try a simpler test
- Don't try to get `vkGetDeviceProcAddr`
- Just try to get other instance functions like `vkCreateDevice`
- See if those work

### 5. Check if the issue is with the test harness
- Try running `vulkan_debug_harness` through the test harness
- See if it crashes the same way

## What We Should NOT Try

- **Don't try to fix the `dlsym` approach** — we've already tried many variations and they all failed
- **Don't try to use a different Vulkan loader** — the Vulkan loader is working fine on the host
- **Don't try to use a different ICD** — the ICD is working fine on the host

## Summary

The issue is that `dlsym` is returning a function pointer that crashes when we call it. We've tried many approaches to fix this, but none have worked. The most likely explanation is that the function we're getting from `dlsym` is not actually `vkGetInstanceProcAddr`, or that there's something wrong with how we're using it.

The next step should be to verify what function we're actually getting from `dlsym`, and then try using the global `vkGetInstanceProcAddr` directly instead of the one from `dlsym`.
