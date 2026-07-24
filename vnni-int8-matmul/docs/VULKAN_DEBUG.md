# Vulkan Backend Debug Guide

## Problem

The Vulkan backend in `vnni-int8-matmul/gpu/vulkan_backend.c` fails to load on Strix Halo headless systems because:

1. **Static Vulkan entry points**: `vkCreateInstance` and `vkDestroyInstance` are static entry points that must be loaded via `dlsym()`, not `vkGetInstanceProcAddr()`
2. **Headless environment**: Without a display server, `vkEnumeratePhysicalDevices` may return 0 devices unless `VK_ICD_FILENAMES` is set

## Solution

### 1. Fix Vulkan Loading (Already Applied)

The `load_dispatch()` function now loads static entry points correctly:

```c
// Static entry points via dlsym
dispatch->vkCreateInstance = (PFN_vkCreateInstance)dlsym(handle, "vkCreateInstance");
dispatch->vkDestroyInstance = (PFN_vkDestroyInstance)dlsym(handle, "vkDestroyInstance");

// All other functions via vkGetInstanceProcAddr
dispatch->vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)get_proc_addr(NULL, "vkEnumeratePhysicalDevices");
// ... rest of the functions
```

### 2. Set VK_ICD_FILENAMES for Headless Systems

For headless environments (containers, CI runners), set the Vulkan ICD file:

```bash
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json
```

This tells the Vulkan loader to use only the RADV driver, avoiding crashes from other ICDs.

### 3. Test Locally on Strix Halo

```bash
cd /home/leite/colibri/vnni-int8-matmul

# Build
make clean && make

# Run tests (Vulkan will work if VK_ICD_FILENAMES is set)
make test
```

Expected output:
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

### 4. Test in Podman Container

```bash
podman run -it --rm \
  --device /dev/dri \
  --device /dev/kfd \
  --security-opt label=disable \
  --cap-add=SYS_PTRACE \
  -v /home/leite/colibri/vnni-int8-matmul:/opt/vnni:rw \
  -e VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json \
  docker.io/kyuz0/amd-strix-halo-toolboxes:vulkan-radv \
  bash -c 'cd /opt/vnni && ln -sf /usr/bin/ld.bfd /etc/alternatives/ld && ln -sf /usr/bin/ld.bfd /usr/bin/ld && make clean && make && make test'
```

## Debugging Steps

If tests still skip, check:

1. **Vulkan library loads**:
   ```bash
   ldd ./tests/test_backends | grep vulkan
   ```

2. **Physical devices enumerate**:
   ```bash
   vulkaninfo --summary | grep -A5 "GPU0"
   ```

3. **Backend name**:
   ```bash
   ./tests/test_backends 2>&1 | grep "Vulkan backend"
   # Should say "OK (vulkan-compute)" not "SKIP"
   ```

4. **Container GPU access**:
   ```bash
   podman exec <container> ls -la /dev/dri/
   # Should show card0 and renderD128
   ```

## Common Errors

### "vkDestroyInstance not found"
**Fix**: Use `dlsym()` instead of `vkGetInstanceProcAddr()` for static entry points.

### "vkEnumeratePhysicalDevices returns 0"
**Fix**: Set `VK_ICD_FILENAMES` environment variable.

### "ld: cannot find"
**Fix**: Create symlink: `ln -sf /usr/bin/ld.bfd /etc/alternatives/ld`

### "Segmentation fault"
**Fix**: Ensure `VK_ICD_FILENAMES` is set to avoid loading incompatible ICDs.

## CI Integration

See `docs/CI_TESTING.md` for GitHub Actions workflow to test on Strix Halo.
