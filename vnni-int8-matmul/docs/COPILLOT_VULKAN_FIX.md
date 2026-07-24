# Vulkan Backend Fix for Copilot

## Problem Summary

The Vulkan backend in `vnni-int8-matmul/gpu/vulkan_backend.c` fails to initialize on Strix Halo systems due to incorrect Vulkan function loading. The backend reports "vulkan-unavailable" and tests skip.

## Root Cause

The original code attempted to load all Vulkan functions via `vkGetInstanceProcAddr()`, but **most Vulkan functions are not available through this API on this system**. They must be loaded via `dlsym()` instead.

### Functions That Must Use dlsym() (NOT vkGetInstanceProcAddr)

The following functions are **static entry points** and must be loaded via `dlsym()`:

- `vkCreateInstance`
- `vkDestroyInstance`
- `vkEnumeratePhysicalDevices`
- `vkGetPhysicalDeviceProperties`
- `vkGetPhysicalDeviceQueueFamilyProperties`
- `vkGetPhysicalDeviceMemoryProperties`
- `vkCreateDevice`
- `vkDestroyDevice`
- `vkGetDeviceQueue`
- `vkCreateCommandPool`
- `vkDestroyCommandPool`
- `vkAllocateCommandBuffers`
- `vkFreeCommandBuffers`
- `vkBeginCommandBuffer`
- `vkEndCommandBuffer`
- `vkCreateDescriptorSetLayout`
- `vkDestroyDescriptorSetLayout`
- `vkCreateShaderModule`
- `vkDestroyShaderModule`
- `vkCreatePipelineLayout`
- `vkDestroyPipelineLayout`
- `vkCreateComputePipelines`
- `vkDestroyPipeline`
- `vkCreateDescriptorPool`
- `vkDestroyDescriptorPool`
- `vkAllocateDescriptorSets`
- `vkUpdateDescriptorSets`
- `vkCreateBuffer`
- `vkDestroyBuffer`
- `vkGetBufferMemoryRequirements`
- `vkAllocateMemory`
- `vkFreeMemory`
- `vkBindBufferMemory`
- `vkMapMemory`
- `vkUnmapMemory`
- `vkCmdBindPipeline`
- `vkCmdBindDescriptorSets`
- `vkCmdPushConstants`
- `vkCmdDispatch`
- `vkQueueSubmit`
- `vkWaitForFences`
- `vkCreateFence`
- `vkDestroyFence`

**Only `vkGetInstanceProcAddr` itself** should be loaded via `dlsym()` and used to get other function pointers (but in this case, we don't need it at all).

## Solution

### Step 1: Replace All Function Loading

In the `load_dispatch()` function, replace all instances of:

```c
dispatch->vkFunctionName = (PFN_vkFunctionName)get_proc_addr(NULL, "vkFunctionName");
```

With:

```c
dispatch->vkFunctionName = (PFN_vkFunctionName)dlsym(handle, "vkFunctionName");
```

### Step 2: Remove Unused Variables

Remove the `get_proc_addr` variable and its initialization:

```c
// DELETE THIS:
PFN_vkGetInstanceProcAddr get_proc_addr = (PFN_vkGetInstanceProcAddr)dlsym(handle, "vkGetInstanceProcAddr");
if (!get_proc_addr) {
    dlclose(handle);
    g_vulkan_handle = NULL;
    return 0;
}
```

### Step 3: Fix API Version

Change the API version from `VK_API_VERSION_1_2` to `0`:

```c
// BEFORE:
app_info.apiVersion = VK_API_VERSION_1_2;

// AFTER:
app_info.apiVersion = 0;
```

This is required because the Vulkan loader rejects API versions that are not 0 or >= VK_API_VERSION_1_0.

## Implementation

### sed Commands

Apply these changes to `gpu/vulkan_backend.c`:

```bash
# Replace all get_proc_addr calls with dlsym calls
sed -i 's|(PFN_vk\([A-Za-z]*\))get_proc_addr(NULL, "vk\([A-Za-z]*\)")|(PFN_vk\1)dlsym(handle, "vk\2")|g' gpu/vulkan_backend.c

# Fix API version
sed -i 's|app_info.apiVersion = VK_API_VERSION_1_2;|app_info.apiVersion = 0;|' gpu/vulkan_backend.c
```

### Manual Fix (Recommended)

1. Open `gpu/vulkan_backend.c`
2. Find the `load_dispatch()` function (around line 100)
3. Replace ALL lines that use `get_proc_addr(NULL, "vk...")` with `dlsym(handle, "vk...")`
4. Remove the `get_proc_addr` variable declaration and check (lines 109-113)
5. Change `app_info.apiVersion = VK_API_VERSION_1_2;` to `app_info.apiVersion = 0;` (line 174)
6. Save and rebuild

## Testing

### Build

```bash
cd vnni-int8-matmul
make clean && make
```

### Run Tests

```bash
make test
```

### Expected Output

```
./tests/test_backends
CPU backend OK (avx512-vnni)
Vulkan backend OK (vulkan-compute)
XDNA2 backend OK (xdna2-fixed-4x1)
Edge-case tests OK
All backend tests passed.

./tests/vulkan_runtime_test
Vulkan runtime test passed via vulkan-compute
```

### If Tests Still Skip

1. Check if Vulkan library loads:
   ```bash
   ldd ./tests/test_backends | grep vulkan
   ```

2. Check if physical devices enumerate:
   ```bash
   vulkaninfo --summary | grep -A5 "GPU0"
   ```

3. For headless environments, set:
   ```bash
   export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json
   ```

## Common Errors

### "vkDestroyInstance not found"

**Cause**: Function loaded via `vkGetInstanceProcAddr()` instead of `dlsym()`

**Fix**: Use `dlsym()` for all Vulkan functions

### "vkEnumeratePhysicalDevices not found"

**Cause**: Same as above

**Fix**: Use `dlsym()` for all Vulkan functions

### "apiVersion has value of 131072 which is not permitted"

**Cause**: API version set to `VK_API_VERSION_1_2` (131072)

**Fix**: Set to `0` or `VK_API_VERSION_1_0`

### "ld: cannot find"

**Cause**: Missing linker in container

**Fix**: Create symlink:
```bash
ln -sf /usr/bin/ld.bfd /etc/alternatives/ld
ln -sf /usr/bin/ld.bfd /usr/bin/ld
```

## Verification

After applying the fix, verify by checking the backend name:

```c
const char *name = strix_vulkan_backend_name();
// Should return "vulkan-compute" not "vulkan-unavailable"
```

## Files Modified

- `vnni-int8-matmul/gpu/vulkan_backend.c` — Fixed function loading and API version

## References

- Vulkan Spec: https://www.khronos.org/vulkan/
- Vulkan Loader: https://github.com/KhronosGroup/Vulkan-Loader
- RADV Driver: https://docs.mesa3d.org/drivers/radv.html

## Next Steps

1. Apply the fix to `gpu/vulkan_backend.c`
2. Build and test on Strix Halo
3. If tests pass, commit and push
4. Update documentation in `docs/VULKAN_DEBUG.md`
5. Test in CI/CD environment

## Notes

- The `get_proc_addr` variable is no longer needed and can be removed
- All Vulkan functions in this backend must use `dlsym()` for loading
- The API version must be 0 to comply with Vulkan loader requirements
- Headless environments require `VK_ICD_FILENAMES` environment variable
