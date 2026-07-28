/**
 * vulkan_backend.c — headless Vulkan compute backend for AMD Strix Halo.
 *
 * Target: AMD Ryzen AI Max+ 395 "Strix Halo" iGPU (Radeon 8060S / 8050S,
 *         RADV GFX1151) running headless inside a Podman container.
 *
 * Design rules (see docs/vulkan_debug_attempts.md, "Guardrails"):
 *
 *  1. All Vulkan types come from the official <vulkan/vulkan_core.h>. This
 *     translation unit must never declare a Vulkan struct, enum or handle.
 *  2. Only `vkGetInstanceProcAddr` is resolved with dlsym(); every other entry
 *     point is resolved through the vkGetInstanceProcAddr / vkGetDeviceProcAddr
 *     chain, as required by the Vulkan Loader Interface.
 *  3. Headless only. No surface, swapchain, WSI or display extension is
 *     requested or required. No DISPLAY, WAYLAND_DISPLAY or Xvfb is needed.
 *  4. Strix Halo only. If the running machine does not expose a Strix Halo
 *     iGPU with a compute queue, initialisation fails loudly. There is no
 *     fallback to a discrete GPU, to lavapipe/llvmpipe, or to the CPU.
 */

#include "vulkan_backend.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vulkan_dispatch.h"

/* Forward declarations for functions called before they are defined. */
static int select_host_memory_type(StrixVulkanContext *ctx, uint32_t type_bits, uint32_t *out_index);
static void destroy_buffer(StrixVulkanContext *ctx, StrixBuffer *b);
static int create_buffer(StrixVulkanContext *ctx, VkDeviceSize size, StrixBuffer *b);


/* Push constant block consumed by gpu/comp.spv (3 x uint32 at offsets 0/4/8). */
typedef struct {
    uint32_t rows;
    uint32_t cols;
    uint32_t inner;
} StrixPushConstants;

/* ── Buffers ─────────────────────────────────────────────────────────────── */

typedef struct {
    VkBuffer       buffer;
    VkDeviceMemory memory;
    VkDeviceSize   size;
} StrixBuffer;

/* Per-shape cached buffers so we do not re-allocate on every dispatch. */
typedef struct {
    int32_t                      rows;
    int32_t                      inner_dim;
    int32_t                      out_cols;
    StrixBuffer                  buffers[3];
    size_t                       a_bytes;
    size_t                       b_bytes;
    size_t                       c_bytes;
} CachedBuffers;

#define MAX_CACHED 16

typedef struct {
    void                        *loader;
    VkInstance                   instance;
    VkPhysicalDevice             physical_device;
    VkDevice                     device;
    VkQueue                      queue;
    uint32_t                     queue_family_index;
    uint32_t                     memory_type_index_host;
    VkCommandPool                command_pool;
    VkCommandBuffer              command_buffer;
    VkDescriptorSetLayout        descriptor_layout;
    VkDescriptorSet              descriptor_set;
    VkShaderModule               shader_module;
    VkPipelineLayout             pipeline_layout;
    VkPipeline                   pipeline;
    VkFence                      fence;
    char                         device_name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    VnnVulkanInstanceDispatch    vk;
    VnnVulkanDeviceDispatch      dev;
    bool                         ready;
    CachedBuffers                cached[MAX_CACHED];
    int                          cached_count;
} StrixVulkanContext;

/* Lazily created, process-wide. Creating a VkDevice per matmul call costs tens
 * of milliseconds, which dwarfs the dispatch itself. */
static StrixVulkanContext g_ctx;
static int g_init_attempted = 0;
static const char *g_failure_reason = "not initialised";

/* ── Diagnostics ─────────────────────────────────────────────────────────── */

static void vk_debugf(const char *fmt, ...) {
    const char *enabled = getenv("VNNI_VULKAN_DEBUG");
    if (!enabled || !enabled[0] || strcmp(enabled, "0") == 0) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "[vulkan] ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

static int vk_fail(const char *reason) {
    g_failure_reason = reason;
    vk_debugf("initialisation failed: %s", reason);
    return 0;
}

const char *strix_vulkan_failure_reason(void) {
    return g_failure_reason;
}

/* ── Strix Halo identification ───────────────────────────────────────────── */

#define VNNI_PCI_VENDOR_AMD 0x1002u

static int name_contains_ci(const char *haystack, const char *needle) {
    size_t nlen = strlen(needle);
    if (nlen == 0) return 0;
    for (const char *p = haystack; *p; ++p) {
        size_t i = 0;
        while (i < nlen) {
            char a = p[i];
            char b = needle[i];
            if (a >= 'a' && a <= 'z') a = (char)(a - 'a' + 'A');
            if (b >= 'a' && b <= 'z') b = (char)(b - 'a' + 'A');
            if (a != b) break;
            ++i;
        }
        if (i == nlen) return 1;
    }
    return 0;
}

/*
 * Strix Halo exposes its iGPU through RADV as an INTEGRATED_GPU with AMD's PCI
 * vendor id and a device name that carries either the RADV GPU generation
 * ("GFX1151") or the marketing name ("Radeon 8060S" / "Radeon 8050S").
 *
 * VNNI_STRIX_DEVICE_NAME may add one more accepted substring so that a future
 * Strix Halo SKU name can be allowlisted. It cannot be used to select a
 * non-AMD or non-integrated device: those checks are unconditional.
 */
static int is_strix_halo_device(const VkPhysicalDeviceProperties *props) {
    if (props->vendorID != VNNI_PCI_VENDOR_AMD) {
        vk_debugf("rejecting device '%s': vendorID 0x%04x is not AMD",
                  props->deviceName, props->vendorID);
        return 0;
    }
    if (props->deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        vk_debugf("rejecting device '%s': deviceType %d is not INTEGRATED_GPU",
                  props->deviceName, (int)props->deviceType);
        return 0;
    }
    if (name_contains_ci(props->deviceName, "GFX1151") ||
        name_contains_ci(props->deviceName, "8060S") ||
        name_contains_ci(props->deviceName, "8050S")) {
        return 1;
    }
    const char *extra = getenv("VNNI_STRIX_DEVICE_NAME");
    if (extra && extra[0] && name_contains_ci(props->deviceName, extra)) {
        vk_debugf("accepting device '%s' via VNNI_STRIX_DEVICE_NAME", props->deviceName);
        return 1;
    }
    vk_debugf("rejecting device '%s': not a recognised Strix Halo iGPU",
              props->deviceName);
    return 0;
}

/* ── SPIR-V loading ──────────────────────────────────────────────────────── */

static const char *shader_path(void) {
    const char *override_path = getenv("VNNI_VULKAN_SHADER");
    if (override_path && override_path[0]) {
        return override_path;
    }
    /* Run from the project root (`make test`) or from tests/. */
    static const char *candidates[] = {"gpu/comp.spv", "../gpu/comp.spv"};
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        FILE *fp = fopen(candidates[i], "rb");
        if (fp) {
            fclose(fp);
            return candidates[i];
        }
    }
    return candidates[0];
}

static int read_shader_binary(const char *path, uint32_t **code, size_t *code_size) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        vk_debugf("cannot open SPIR-V module '%s'", path);
        return 0;
    }
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0; }
    long size = ftell(fp);
    if (size <= 0 || (size % 4) != 0) {
        vk_debugf("SPIR-V module '%s' has invalid size %ld (must be a non-zero multiple of 4)",
                  path, size);
        fclose(fp);
        return 0;
    }
    rewind(fp);
    uint32_t *buffer = (uint32_t *)malloc((size_t)size);
    if (!buffer) { fclose(fp); return 0; }
    if (fread(buffer, 1, (size_t)size, fp) != (size_t)size) {
        free(buffer);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    if (buffer[0] != 0x07230203u) {
        vk_debugf("SPIR-V module '%s' has bad magic 0x%08x", path, buffer[0]);
        free(buffer);
        return 0;
    }
    *code = buffer;
    *code_size = (size_t)size;
    return 1;
}

/* ── Loader / dispatch resolution ────────────────────────────────────────── */

static int load_instance_dispatch(StrixVulkanContext *ctx) {
    static const char *candidates[] = {"libvulkan.so.1", "libvulkan.so"};
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        dlerror();
        ctx->loader = dlopen(candidates[i], RTLD_NOW | RTLD_LOCAL);
        if (ctx->loader) {
            vk_debugf("loaded %s", candidates[i]);
            break;
        }
        vk_debugf("dlopen(%s) failed: %s", candidates[i], dlerror());
    }
    if (!ctx->loader) {
        return vk_fail("libvulkan.so.1 not found (install the Vulkan loader in the container)");
    }

    /* The loader spec guarantees exactly one exported symbol is safe to take
     * via dlsym: vkGetInstanceProcAddr. */
    ctx->vk.vkGetInstanceProcAddr =
        (PFN_vkGetInstanceProcAddr)dlsym(ctx->loader, "vkGetInstanceProcAddr");
    if (!ctx->vk.vkGetInstanceProcAddr) {
        return vk_fail("libvulkan does not export vkGetInstanceProcAddr");
    }

    ctx->vk.vkCreateInstance =
        (PFN_vkCreateInstance)ctx->vk.vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    ctx->vk.vkEnumerateInstanceVersion =
        (PFN_vkEnumerateInstanceVersion)ctx->vk.vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");
    if (!ctx->vk.vkCreateInstance) {
        return vk_fail("vkGetInstanceProcAddr(NULL, \"vkCreateInstance\") returned NULL");
    }
    return 1;
}

#define RESOLVE_INSTANCE_PROC(ctx, name)                                            \
    do {                                                                            \
        (ctx)->vk.name = (PFN_##name)(ctx)->vk.vkGetInstanceProcAddr((ctx)->instance, #name); \
        if (!(ctx)->vk.name) {                                                      \
            return vk_fail("missing instance entry point " #name);                  \
        }                                                                           \
    } while (0)

static int resolve_instance_procs(StrixVulkanContext *ctx) {
    RESOLVE_INSTANCE_PROC(ctx, vkDestroyInstance);
    RESOLVE_INSTANCE_PROC(ctx, vkEnumeratePhysicalDevices);
    RESOLVE_INSTANCE_PROC(ctx, vkGetPhysicalDeviceProperties);
    RESOLVE_INSTANCE_PROC(ctx, vkGetPhysicalDeviceQueueFamilyProperties);
    RESOLVE_INSTANCE_PROC(ctx, vkGetPhysicalDeviceMemoryProperties);
    RESOLVE_INSTANCE_PROC(ctx, vkCreateDevice);
    RESOLVE_INSTANCE_PROC(ctx, vkGetDeviceProcAddr);
    return 1;
}

#define RESOLVE_DEVICE_PROC(ctx, name)                                              \
    do {                                                                            \
        (ctx)->dev.name = (PFN_##name)(ctx)->vk.vkGetDeviceProcAddr((ctx)->device, #name); \
        if (!(ctx)->dev.name) {                                                     \
            return vk_fail("missing device entry point " #name);                    \
        }                                                                           \
    } while (0)

static int resolve_device_procs(StrixVulkanContext *ctx) {
    RESOLVE_DEVICE_PROC(ctx, vkDestroyDevice);
    RESOLVE_DEVICE_PROC(ctx, vkDeviceWaitIdle);
    RESOLVE_DEVICE_PROC(ctx, vkGetDeviceQueue);
    RESOLVE_DEVICE_PROC(ctx, vkCreateCommandPool);
    RESOLVE_DEVICE_PROC(ctx, vkDestroyCommandPool);
    RESOLVE_DEVICE_PROC(ctx, vkAllocateCommandBuffers);
    RESOLVE_DEVICE_PROC(ctx, vkFreeCommandBuffers);
    RESOLVE_DEVICE_PROC(ctx, vkBeginCommandBuffer);
    RESOLVE_DEVICE_PROC(ctx, vkEndCommandBuffer);
    RESOLVE_DEVICE_PROC(ctx, vkResetCommandBuffer);
    RESOLVE_DEVICE_PROC(ctx, vkCreateDescriptorSetLayout);
    RESOLVE_DEVICE_PROC(ctx, vkDestroyDescriptorSetLayout);
    RESOLVE_DEVICE_PROC(ctx, vkCreateShaderModule);
    RESOLVE_DEVICE_PROC(ctx, vkDestroyShaderModule);
    RESOLVE_DEVICE_PROC(ctx, vkCreatePipelineLayout);
    RESOLVE_DEVICE_PROC(ctx, vkDestroyPipelineLayout);
    RESOLVE_DEVICE_PROC(ctx, vkCreateComputePipelines);
    RESOLVE_DEVICE_PROC(ctx, vkDestroyPipeline);
    RESOLVE_DEVICE_PROC(ctx, vkCreateDescriptorPool);
    RESOLVE_DEVICE_PROC(ctx, vkDestroyDescriptorPool);
    RESOLVE_DEVICE_PROC(ctx, vkAllocateDescriptorSets);
    RESOLVE_DEVICE_PROC(ctx, vkUpdateDescriptorSets);
    RESOLVE_DEVICE_PROC(ctx, vkCreateBuffer);
    RESOLVE_DEVICE_PROC(ctx, vkDestroyBuffer);
    RESOLVE_DEVICE_PROC(ctx, vkGetBufferMemoryRequirements);
    RESOLVE_DEVICE_PROC(ctx, vkAllocateMemory);
    RESOLVE_DEVICE_PROC(ctx, vkFreeMemory);
    RESOLVE_DEVICE_PROC(ctx, vkBindBufferMemory);
    RESOLVE_DEVICE_PROC(ctx, vkMapMemory);
    RESOLVE_DEVICE_PROC(ctx, vkUnmapMemory);
    RESOLVE_DEVICE_PROC(ctx, vkCmdBindPipeline);
    RESOLVE_DEVICE_PROC(ctx, vkCmdBindDescriptorSets);
    RESOLVE_DEVICE_PROC(ctx, vkCmdPushConstants);
    RESOLVE_DEVICE_PROC(ctx, vkCmdPipelineBarrier);
    RESOLVE_DEVICE_PROC(ctx, vkCmdDispatch);
    RESOLVE_DEVICE_PROC(ctx, vkQueueSubmit);
    RESOLVE_DEVICE_PROC(ctx, vkCreateFence);
    RESOLVE_DEVICE_PROC(ctx, vkDestroyFence);
    RESOLVE_DEVICE_PROC(ctx, vkWaitForFences);
    RESOLVE_DEVICE_PROC(ctx, vkResetFences);
    return 1;
}

/* ── Teardown ────────────────────────────────────────────────────────────── */

static void destroy_context(StrixVulkanContext *ctx) {
    /* Free any cached per-shape buffers. */
    for (int i = 0; i < ctx->cached_count; ++i) {
        for (int j = 0; j < 3; ++j) {
            destroy_buffer(ctx, &ctx->cached[i].buffers[j]);
        }
    }
    ctx->cached_count = 0;

    if (ctx->device) {
        if (ctx->dev.vkDeviceWaitIdle) ctx->dev.vkDeviceWaitIdle(ctx->device);
        if (ctx->fence)             ctx->dev.vkDestroyFence(ctx->device, ctx->fence, NULL);
        if (ctx->pipeline)          ctx->dev.vkDestroyPipeline(ctx->device, ctx->pipeline, NULL);
        if (ctx->pipeline_layout)   ctx->dev.vkDestroyPipelineLayout(ctx->device, ctx->pipeline_layout, NULL);
        if (ctx->shader_module)     ctx->dev.vkDestroyShaderModule(ctx->device, ctx->shader_module, NULL);
        if (ctx->descriptor_pool)   ctx->dev.vkDestroyDescriptorPool(ctx->device, ctx->descriptor_pool, NULL);
        if (ctx->descriptor_layout) ctx->dev.vkDestroyDescriptorSetLayout(ctx->device, ctx->descriptor_layout, NULL);
        if (ctx->command_pool) {
            ctx->dev.vkFreeCommandBuffers(ctx->device, ctx->command_pool, STRIX_VULKAN_MAX_BATCH, ctx->command_buffers);
            ctx->dev.vkDestroyCommandPool(ctx->device, ctx->command_pool, NULL);
        }
        ctx->dev.vkDestroyDevice(ctx->device, NULL);
    }
    if (ctx->instance && ctx->vk.vkDestroyInstance) {
        ctx->vk.vkDestroyInstance(ctx->instance, NULL);
    }
    if (ctx->loader) {
        dlclose(ctx->loader);
    }
    memset(ctx, 0, sizeof(*ctx));
}

/* ── Device selection ────────────────────────────────────────────────────── */

static int select_strix_device(StrixVulkanContext *ctx) {
    uint32_t device_count = 0;
    if (ctx->vk.vkEnumeratePhysicalDevices(ctx->instance, &device_count, NULL) != VK_SUCCESS ||
        device_count == 0) {
        return vk_fail("no Vulkan physical device (is /dev/dri passed into the container?)");
    }

    VkPhysicalDevice *devices =
        (VkPhysicalDevice *)calloc(device_count, sizeof(VkPhysicalDevice));
    if (!devices) {
        return vk_fail("out of memory enumerating physical devices");
    }
    if (ctx->vk.vkEnumeratePhysicalDevices(ctx->instance, &device_count, devices) != VK_SUCCESS) {
        free(devices);
        return vk_fail("vkEnumeratePhysicalDevices failed");
    }

    int found = 0;
    for (uint32_t i = 0; i < device_count && !found; ++i) {
        /* Full-size, zero-initialised: VkPhysicalDeviceProperties is 824 bytes
         * and the driver writes all of it. */
        VkPhysicalDeviceProperties props;
        memset(&props, 0, sizeof(props));
        ctx->vk.vkGetPhysicalDeviceProperties(devices[i], &props);
        vk_debugf("physical device %u: '%s' vendor=0x%04x device=0x%04x type=%d api=%u.%u.%u",
                  i, props.deviceName, props.vendorID, props.deviceID, (int)props.deviceType,
                  VK_API_VERSION_MAJOR(props.apiVersion),
                  VK_API_VERSION_MINOR(props.apiVersion),
                  VK_API_VERSION_PATCH(props.apiVersion));

        if (!is_strix_halo_device(&props)) {
            continue;
        }

        uint32_t family_count = 0;
        ctx->vk.vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &family_count, NULL);
        if (family_count == 0) {
            continue;
        }
        VkQueueFamilyProperties *families =
            (VkQueueFamilyProperties *)calloc(family_count, sizeof(VkQueueFamilyProperties));
        if (!families) {
            free(devices);
            return vk_fail("out of memory querying queue families");
        }
        ctx->vk.vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &family_count, families);
        for (uint32_t j = 0; j < family_count; ++j) {
            if ((families[j].queueFlags & VK_QUEUE_COMPUTE_BIT) && families[j].queueCount > 0) {
                ctx->physical_device = devices[i];
                ctx->queue_family_index = j;
                memcpy(ctx->device_name, props.deviceName, sizeof(ctx->device_name));
                ctx->device_name[sizeof(ctx->device_name) - 1] = '\0';
                found = 1;
                vk_debugf("selected '%s', compute queue family %u", ctx->device_name, j);
                break;
            }
        }
        free(families);
    }

    free(devices);
    if (!found) {
        return vk_fail("no Strix Halo iGPU with a compute queue was found; "
                       "this backend is Strix Halo exclusive and has no fallback");
    }
    return 1;
}

static int select_host_memory_type(StrixVulkanContext *ctx, uint32_t type_bits, uint32_t *out_index) {
    VkPhysicalDeviceMemoryProperties props;
    memset(&props, 0, sizeof(props));
    ctx->vk.vkGetPhysicalDeviceMemoryProperties(ctx->physical_device, &props);

    const VkMemoryPropertyFlags required =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    /* Strix Halo is a unified-memory part: DEVICE_LOCAL | HOST_VISIBLE is the
     * fast path and avoids a staging copy entirely. Prefer it. */
    const VkMemoryPropertyFlags preferred = required | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    for (uint32_t pass = 0; pass < 2; ++pass) {
        VkMemoryPropertyFlags want = (pass == 0) ? preferred : required;
        for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
            if (!(type_bits & (1u << i))) continue;
            if ((props.memoryTypes[i].propertyFlags & want) == want) {
                *out_index = i;
                return 1;
            }
        }
    }
    return 0;
}

/* ── Context creation ────────────────────────────────────────────────────── */

static int create_context(StrixVulkanContext *ctx) {
    memset(ctx, 0, sizeof(*ctx));

    if (!load_instance_dispatch(ctx)) {
        return 0;
    }

    VkApplicationInfo app_info;
    memset(&app_info, 0, sizeof(app_info));
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "colibri-vnni";
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.pEngineName = "colibri";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_1;

    /* Headless: zero instance extensions. Requesting VK_KHR_surface here is
     * what forces a display server to exist; we deliberately do not. */
    VkInstanceCreateInfo instance_info;
    memset(&instance_info, 0, sizeof(instance_info));
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;

    VkResult res = ctx->vk.vkCreateInstance(&instance_info, NULL, &ctx->instance);
    if (res != VK_SUCCESS) {
        vk_debugf("vkCreateInstance returned %d", (int)res);
        destroy_context(ctx);
        return vk_fail("vkCreateInstance failed");
    }

    if (!resolve_instance_procs(ctx) || !select_strix_device(ctx)) {
        destroy_context(ctx);
        return 0;
    }

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info;
    memset(&queue_info, 0, sizeof(queue_info));
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = ctx->queue_family_index;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;

    VkDeviceCreateInfo device_info;
    memset(&device_info, 0, sizeof(device_info));
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;

    res = ctx->vk.vkCreateDevice(ctx->physical_device, &device_info, NULL, &ctx->device);
    if (res != VK_SUCCESS) {
        vk_debugf("vkCreateDevice returned %d", (int)res);
        destroy_context(ctx);
        return vk_fail("vkCreateDevice failed");
    }
    if (!resolve_device_procs(ctx)) {
        destroy_context(ctx);
        return 0;
    }
    ctx->dev.vkGetDeviceQueue(ctx->device, ctx->queue_family_index, 0, &ctx->queue);

    VkCommandPoolCreateInfo pool_info;
    memset(&pool_info, 0, sizeof(pool_info));
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = ctx->queue_family_index;
    if (ctx->dev.vkCreateCommandPool(ctx->device, &pool_info, NULL, &ctx->command_pool) != VK_SUCCESS) {
        destroy_context(ctx);
        return vk_fail("vkCreateCommandPool failed");
    }

    VkCommandBufferAllocateInfo cmd_alloc;
    memset(&cmd_alloc, 0, sizeof(cmd_alloc));
    cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc.commandPool = ctx->command_pool;
    cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_alloc.commandBufferCount = STRIX_VULKAN_MAX_BATCH;
    if (ctx->dev.vkAllocateCommandBuffers(ctx->device, &cmd_alloc, ctx->command_buffers) != VK_SUCCESS) {
        destroy_context(ctx);
        return vk_fail("vkAllocateCommandBuffers failed");
    }

    VkDescriptorSetLayoutBinding bindings[3];
    memset(bindings, 0, sizeof(bindings));
    for (uint32_t i = 0; i < 3; ++i) {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    VkDescriptorSetLayoutCreateInfo layout_info;
    memset(&layout_info, 0, sizeof(layout_info));
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 3;
    layout_info.pBindings = bindings;
    if (ctx->dev.vkCreateDescriptorSetLayout(ctx->device, &layout_info, NULL, &ctx->descriptor_layout) != VK_SUCCESS) {
        destroy_context(ctx);
        return vk_fail("vkCreateDescriptorSetLayout failed");
    }

    VkDescriptorPoolSize pool_size;
    memset(&pool_size, 0, sizeof(pool_size));
    pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_size.descriptorCount = 3;
    VkDescriptorPoolCreateInfo desc_pool_info;
    memset(&desc_pool_info, 0, sizeof(desc_pool_info));
    desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    desc_pool_info.maxSets = 1;
    desc_pool_info.poolSizeCount = 1;
    desc_pool_info.pPoolSizes = &pool_size;
    if (ctx->dev.vkCreateDescriptorPool(ctx->device, &desc_pool_info, NULL, &ctx->descriptor_pool) != VK_SUCCESS) {
        destroy_context(ctx);
        return vk_fail("vkCreateDescriptorPool failed");
    }

    VkDescriptorSetAllocateInfo set_alloc;
    memset(&set_alloc, 0, sizeof(set_alloc));
    set_alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    set_alloc.descriptorPool = ctx->descriptor_pool;
    set_alloc.descriptorSetCount = 1;
    set_alloc.pSetLayouts = &ctx->descriptor_layout;
    if (ctx->dev.vkAllocateDescriptorSets(ctx->device, &set_alloc, &ctx->descriptor_set) != VK_SUCCESS) {
        destroy_context(ctx);
        return vk_fail("vkAllocateDescriptorSets failed");
    }

    uint32_t *code = NULL;
    size_t code_size = 0;
    const char *spv = shader_path();
    if (!read_shader_binary(spv, &code, &code_size)) {
        destroy_context(ctx);
        return vk_fail("could not read gpu/comp.spv (set VNNI_VULKAN_SHADER)");
    }
    VkShaderModuleCreateInfo shader_info;
    memset(&shader_info, 0, sizeof(shader_info));
    shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_info.codeSize = code_size;
    shader_info.pCode = code;
    VkResult shader_res = ctx->dev.vkCreateShaderModule(ctx->device, &shader_info, NULL, &ctx->shader_module);
    free(code);
    if (shader_res != VK_SUCCESS) {
        destroy_context(ctx);
        return vk_fail("vkCreateShaderModule failed");
    }

    VkPushConstantRange push_range;
    memset(&push_range, 0, sizeof(push_range));
    push_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_range.offset = 0;
    push_range.size = (uint32_t)sizeof(StrixPushConstants);

    VkPipelineLayoutCreateInfo pl_info;
    memset(&pl_info, 0, sizeof(pl_info));
    pl_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pl_info.setLayoutCount = 1;
    pl_info.pSetLayouts = &ctx->descriptor_layout;
    pl_info.pushConstantRangeCount = 1;
    pl_info.pPushConstantRanges = &push_range;
    if (ctx->dev.vkCreatePipelineLayout(ctx->device, &pl_info, NULL, &ctx->pipeline_layout) != VK_SUCCESS) {
        destroy_context(ctx);
        return vk_fail("vkCreatePipelineLayout failed");
    }

    VkComputePipelineCreateInfo pipeline_info;
    memset(&pipeline_info, 0, sizeof(pipeline_info));
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipeline_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipeline_info.stage.module = ctx->shader_module;
    pipeline_info.stage.pName = "main";
    pipeline_info.layout = ctx->pipeline_layout;
    pipeline_info.basePipelineIndex = -1;
    if (ctx->dev.vkCreateComputePipelines(ctx->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &ctx->pipeline) != VK_SUCCESS) {
        destroy_context(ctx);
        return vk_fail("vkCreateComputePipelines failed");
    }

    VkFenceCreateInfo fence_info;
    memset(&fence_info, 0, sizeof(fence_info));
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (ctx->dev.vkCreateFence(ctx->device, &fence_info, NULL, &ctx->fence) != VK_SUCCESS) {
        destroy_context(ctx);
        return vk_fail("vkCreateFence failed");
    }

    ctx->ready = true;
    g_failure_reason = "none";
    vk_debugf("headless Strix Halo compute context ready on '%s' (SPIR-V: %s)",
              ctx->device_name, spv);
    return 1;
}

static int ensure_context(void) {
    if (!g_init_attempted) {
        g_init_attempted = 1;
        create_context(&g_ctx);
    }
    return g_ctx.ready ? 1 : 0;
}

/* ── Buffers ─────────────────────────────────────────────────────────────── */

static void destroy_buffer(StrixVulkanContext *ctx, StrixBuffer *b) {
    if (b->buffer) ctx->dev.vkDestroyBuffer(ctx->device, b->buffer, NULL);
    if (b->memory) ctx->dev.vkFreeMemory(ctx->device, b->memory, NULL);
    memset(b, 0, sizeof(*b));
}

static int create_buffer(StrixVulkanContext *ctx, VkDeviceSize size, StrixBuffer *b) {
    memset(b, 0, sizeof(*b));
    b->size = size;

    VkBufferCreateInfo info;
    memset(&info, 0, sizeof(info));
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (ctx->dev.vkCreateBuffer(ctx->device, &info, NULL, &b->buffer) != VK_SUCCESS) {
        vk_debugf("vkCreateBuffer failed for %llu bytes", (unsigned long long)size);
        return 0;
    }

    /* VkMemoryRequirements is {size, alignment, memoryTypeBits} — field order
     * matters and is taken from the official header. */
    VkMemoryRequirements reqs;
    memset(&reqs, 0, sizeof(reqs));
    ctx->dev.vkGetBufferMemoryRequirements(ctx->device, b->buffer, &reqs);

    uint32_t type_index = 0;
    if (!select_host_memory_type(ctx, reqs.memoryTypeBits, &type_index)) {
        vk_debugf("no HOST_VISIBLE|HOST_COHERENT memory type for typeBits 0x%08x",
                  reqs.memoryTypeBits);
        destroy_buffer(ctx, b);
        return 0;
    }

    VkMemoryAllocateInfo alloc;
    memset(&alloc, 0, sizeof(alloc));
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.allocationSize = reqs.size;
    alloc.memoryTypeIndex = type_index;
    if (ctx->dev.vkAllocateMemory(ctx->device, &alloc, NULL, &b->memory) != VK_SUCCESS) {
        vk_debugf("vkAllocateMemory failed for %llu bytes", (unsigned long long)reqs.size);
        destroy_buffer(ctx, b);
        return 0;
    }
    if (ctx->dev.vkBindBufferMemory(ctx->device, b->buffer, b->memory, 0) != VK_SUCCESS) {
        vk_debugf("vkBindBufferMemory failed");
        destroy_buffer(ctx, b);
        return 0;
    }
    return 1;
}

/* ── Dispatch ────────────────────────────────────────────────────────────── */

/*
 * Look up or create cached buffers for a given shape.  On the second+ call
 * with the same (rows, inner, cols) we reuse the existing VkBuffer objects,
 * avoiding the ~microsecond vkCreateBuffer + vkAllocateMemory + vkBindBuffer
 * sequence that dominates per-call overhead when the actual dispatch itself
 * is sub-millisecond.
 */
static CachedBuffers *find_or_create_cache(StrixVulkanContext *ctx,
                                           int32_t rows, int32_t inner_dim,
                                           int32_t out_cols) {
    /* 1) Search existing entries for a matching shape. */
    for (int i = 0; i < ctx->cached_count; ++i) {
        CachedBuffers *cb = &ctx->cached[i];
        if (cb->rows == rows && cb->inner_dim == inner_dim && cb->out_cols == out_cols) {
            return cb;
        }
    }

    /* 2) Evict if we are full.  The least recently used slot is the last one
     *    — simplest policy and good enough for the small shape table. */
    int slot = ctx->cached_count;
    if (slot >= MAX_CACHED) {
        slot = MAX_CACHED - 1;
        /* Tear down old buffers. */
        for (int j = 0; j < 3; ++j) {
            destroy_buffer(ctx, &ctx->cached[slot].buffers[j]);
        }
    } else {
        ctx->cached_count++;
    }

    CachedBuffers *cb = &ctx->cached[slot];
    cb->rows = rows;
    cb->inner_dim = inner_dim;
    cb->out_cols = out_cols;
    cb->a_bytes = (size_t)rows * (size_t)inner_dim * sizeof(float);
    cb->b_bytes = (size_t)inner_dim * (size_t)out_cols * sizeof(float);
    cb->c_bytes = (size_t)rows * (size_t)out_cols * sizeof(float);

    int ok = create_buffer(ctx, cb->a_bytes, &cb->buffers[0]) &&
             create_buffer(ctx, cb->b_bytes, &cb->buffers[1]) &&
             create_buffer(ctx, cb->c_bytes, &cb->buffers[2]);
    if (!ok) {
        for (int j = 0; j < 3; ++j) {
            destroy_buffer(ctx, &cb->buffers[j]);
        }
        memset(cb, 0, sizeof(*cb));
        return NULL;
    }
    return cb;
}

static int run_matmul(StrixVulkanContext *ctx,
                      const int8_t *input, int rows, int inner_dim,
                      const int8_t *weights, int out_cols,
                      float *output, const float *scales) {
    const size_t a_count = (size_t)rows * (size_t)inner_dim;
    const size_t b_count = (size_t)inner_dim * (size_t)out_cols;
    const size_t c_count = (size_t)rows * (size_t)out_cols;

    /* Use cached buffers when possible; fall back to per-call allocation. */
    CachedBuffers *cached = find_or_create_cache(ctx, rows, inner_dim, out_cols);
    StrixBuffer buffers[3];
    if (cached) {
        buffers[0] = cached->buffers[0];
        buffers[1] = cached->buffers[1];
        buffers[2] = cached->buffers[2];
    } else {
        /* Last-resort allocation for an uncacheable shape. */
        memset(buffers, 0, sizeof(buffers));
        int ok = create_buffer(ctx, a_count * sizeof(float), &buffers[0]) &&
                 create_buffer(ctx, b_count * sizeof(float), &buffers[1]) &&
                 create_buffer(ctx, c_count * sizeof(float), &buffers[2]);
        if (!ok) {
            goto cleanup;
        }
    }

    /* Upload A (row-major [rows][inner]) widened to f32. */
    void *mapped = NULL;
    if (ctx->dev.vkMapMemory(ctx->device, buffers[0].memory, 0, a_count * sizeof(float), 0, &mapped) != VK_SUCCESS) {
        goto cleanup;
    }
    {
        float *dst = (float *)mapped;
        for (size_t i = 0; i < a_count; ++i) {
            dst[i] = (float)input[i];
        }
    }
    ctx->dev.vkUnmapMemory(ctx->device, buffers[0].memory);

    /* Upload B transposed to [inner][out_cols], which is what comp.spv reads. */
    if (ctx->dev.vkMapMemory(ctx->device, buffers[1].memory, 0, b_count * sizeof(float), 0, &mapped) != VK_SUCCESS) {
        goto cleanup;
    }
    {
        float *dst = (float *)mapped;
        for (int o = 0; o < out_cols; ++o) {
            for (int i = 0; i < inner_dim; ++i) {
                dst[(size_t)i * (size_t)out_cols + (size_t)o] =
                    (float)weights[(size_t)o * (size_t)inner_dim + (size_t)i];
            }
        }
    }
    ctx->dev.vkUnmapMemory(ctx->device, buffers[1].memory);

    if (ctx->dev.vkMapMemory(ctx->device, buffers[2].memory, 0, c_count * sizeof(float), 0, &mapped) != VK_SUCCESS) {
        goto cleanup;
    }
    memset(mapped, 0, (size_t)c_count * sizeof(float));
    ctx->dev.vkUnmapMemory(ctx->device, buffers[2].memory);

    VkDescriptorBufferInfo infos[3];
    memset(infos, 0, sizeof(infos));
    VkWriteDescriptorSet writes[3];
    memset(writes, 0, sizeof(writes));
    for (uint32_t i = 0; i < 3; ++i) {
        infos[i].buffer = buffers[i].buffer;
        infos[i].offset = 0;
        infos[i].range = buffers[i].size;
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = ctx->descriptor_set;
        writes[i].dstBinding = i;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[i].pBufferInfo = &infos[i];
    }
    ctx->dev.vkUpdateDescriptorSets(ctx->device, 3, writes, 0, NULL);

    if (ctx->dev.vkResetCommandBuffer(ctx->command_buffer, 0) != VK_SUCCESS) {
        goto cleanup;
    }

    VkCommandBufferBeginInfo begin;
    memset(&begin, 0, sizeof(begin));
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (ctx->dev.vkBeginCommandBuffer(ctx->command_buffer, &begin) != VK_SUCCESS) {
        goto cleanup;
    }

    ctx->dev.vkCmdBindPipeline(ctx->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, ctx->pipeline);
    ctx->dev.vkCmdBindDescriptorSets(ctx->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                     ctx->pipeline_layout, 0, 1, &ctx->descriptor_set, 0, NULL);
    StrixPushConstants push;
    push.rows = (uint32_t)rows;
    push.cols = (uint32_t)out_cols;
    push.inner = (uint32_t)inner_dim;
    ctx->dev.vkCmdPushConstants(ctx->command_buffer, ctx->pipeline_layout,
                                VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);

    /* comp.spv declares local_size = (16, 16, 1); x maps to output column. */
    const uint32_t group_x = ((uint32_t)out_cols + 15u) / 16u;
    const uint32_t group_y = ((uint32_t)rows + 15u) / 16u;
    ctx->dev.vkCmdDispatch(ctx->command_buffer, group_x, group_y, 1);

    /* Make the shader writes visible to a subsequent host read. Required by the
     * spec even for HOST_COHERENT memory. */
    VkMemoryBarrier barrier;
    memset(&barrier, 0, sizeof(barrier));
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    ctx->dev.vkCmdPipelineBarrier(ctx->command_buffer,
                                  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                  VK_PIPELINE_STAGE_HOST_BIT,
                                  0, 1, &barrier, 0, NULL, 0, NULL);

    if (ctx->dev.vkEndCommandBuffer(ctx->command_buffer) != VK_SUCCESS) {
        goto cleanup;
    }

    if (ctx->dev.vkResetFences(ctx->device, 1, &ctx->fence) != VK_SUCCESS) {
        goto cleanup;
    }

    VkSubmitInfo submit;
    memset(&submit, 0, sizeof(submit));
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &ctx->command_buffer;
    if (ctx->dev.vkQueueSubmit(ctx->queue, 1, &submit, ctx->fence) != VK_SUCCESS) {
        vk_debugf("vkQueueSubmit failed");
        goto cleanup;
    }
    /* 10 s is generous for any shape this backend accepts; an infinite wait
     * would turn a hung queue into a hung test run. */
    if (ctx->dev.vkWaitForFences(ctx->device, 1, &ctx->fence, VK_TRUE, 10ull * 1000ull * 1000ull * 1000ull) != VK_SUCCESS) {
        vk_debugf("vkWaitForFences timed out or failed");
        goto cleanup;
    }

    if (ctx->dev.vkMapMemory(ctx->device, buffers[2].memory, 0, c_count * sizeof(float), 0, &mapped) != VK_SUCCESS) {
        goto cleanup;
    }
    memcpy(output, mapped, (size_t)c_count * sizeof(float));
    ctx->dev.vkUnmapMemory(ctx->device, buffers[2].memory);

    if (scales) {
        for (int r = 0; r < rows; ++r) {
            for (int o = 0; o < out_cols; ++o) {
                output[(size_t)r * (size_t)out_cols + (size_t)o] *= scales[o];
            }
        }
    }

cleanup:
    /* Only destroy buffers we freshly allocated; cached ones are reused. */
    if (!cached) {
        for (int i = 0; i < 3; ++i) {
            destroy_buffer(ctx, &buffers[i]);
        }
    }
    return 1;
}

/* ── Public API ──────────────────────────────────────────────────────────── */

int strix_vulkan_is_supported(void) {
    return ensure_context();
}

int strix_vulkan_matmul(const int8_t *input,
                        int rows,
                        int inner_dim,
                        const int8_t *weights,
                        int out_cols,
                        float *output,
                        const float *scales) {
    if (!input || !weights || !output || rows <= 0 || inner_dim <= 0 || out_cols <= 0) {
        return 0;
    }
    if (!ensure_context()) {
        return 0;
    }
    return run_matmul(&g_ctx, input, rows, inner_dim, weights, out_cols, output, scales);
}

/* Maximum batch size supported by strix_vulkan_batch_matmul(). */
#define STRIX_VULKAN_MAX_BATCH 32

/*
 * run_batch_matmul — submit `batch_size` identical compute dispatches with a
 * single vkQueueSubmit() and one fence wait.
 *
 * All batch items share the same A (input) and B (weights) VkBuffers; each
 * has its own C (output) VkBuffer.  A temporary VkDescriptorPool large enough
 * for `batch_size` sets is created, used, and destroyed per call.  Command
 * buffers are allocated from the existing pool (which has
 * VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) and freed on exit.
 */
static int run_batch_matmul(StrixVulkanContext *ctx,
                            const int8_t *input, int rows, int inner_dim,
                            const int8_t *weights, int out_cols,
                            float *output, const float *scales,
                            int batch_size) {
    const size_t a_count = (size_t)rows * (size_t)inner_dim;
    const size_t b_count = (size_t)inner_dim * (size_t)out_cols;
    const size_t c_count = (size_t)rows * (size_t)out_cols;
    const VkDeviceSize a_bytes = (VkDeviceSize)(a_count * sizeof(float));
    const VkDeviceSize b_bytes = (VkDeviceSize)(b_count * sizeof(float));
    const VkDeviceSize c_bytes = (VkDeviceSize)(c_count * sizeof(float));
    int ok = 0;
    void *mapped = NULL;

    /* Shared A and B buffers (input and weights), uploaded once. */
    StrixBuffer buf_a, buf_b;
    memset(&buf_a, 0, sizeof(buf_a));
    memset(&buf_b, 0, sizeof(buf_b));
    if (!create_buffer(ctx, a_bytes, &buf_a) ||
        !create_buffer(ctx, b_bytes, &buf_b)) {
        goto cleanup_ab;
    }

    if (ctx->dev.vkMapMemory(ctx->device, buf_a.memory, 0, a_bytes, 0, &mapped) != VK_SUCCESS) {
        goto cleanup_ab;
    }
    {
        float *dst = (float *)mapped;
        for (size_t i = 0; i < a_count; ++i) dst[i] = (float)input[i];
    }
    ctx->dev.vkUnmapMemory(ctx->device, buf_a.memory);

    if (ctx->dev.vkMapMemory(ctx->device, buf_b.memory, 0, b_bytes, 0, &mapped) != VK_SUCCESS) {
        goto cleanup_ab;
    }
    {
        float *dst = (float *)mapped;
        for (int o = 0; o < out_cols; ++o)
            for (int i = 0; i < inner_dim; ++i)
                dst[(size_t)i * (size_t)out_cols + (size_t)o] =
                    (float)weights[(size_t)o * (size_t)inner_dim + (size_t)i];
    }
    ctx->dev.vkUnmapMemory(ctx->device, buf_b.memory);

    /* Per-batch-item C buffers. */
    StrixBuffer *c_bufs = (StrixBuffer *)calloc((size_t)batch_size, sizeof(StrixBuffer));
    if (!c_bufs) goto cleanup_ab;

    int c_count_alloc = 0;
    for (int b = 0; b < batch_size; ++b) {
        if (!create_buffer(ctx, c_bytes, &c_bufs[b])) goto cleanup_c;
        c_count_alloc = b + 1;
        if (ctx->dev.vkMapMemory(ctx->device, c_bufs[b].memory, 0, c_bytes, 0, &mapped) != VK_SUCCESS) {
            goto cleanup_c;
        }
        memset(mapped, 0, (size_t)c_bytes);
        ctx->dev.vkUnmapMemory(ctx->device, c_bufs[b].memory);
    }

    /* Temporary descriptor pool large enough for `batch_size` sets. */
    VkDescriptorPoolSize pool_size;
    memset(&pool_size, 0, sizeof(pool_size));
    pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_size.descriptorCount = 3u * (uint32_t)batch_size;
    VkDescriptorPoolCreateInfo desc_pool_info;
    memset(&desc_pool_info, 0, sizeof(desc_pool_info));
    desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    desc_pool_info.maxSets = (uint32_t)batch_size;
    desc_pool_info.poolSizeCount = 1;
    desc_pool_info.pPoolSizes = &pool_size;
    VkDescriptorPool temp_pool = VK_NULL_HANDLE;
    if (ctx->dev.vkCreateDescriptorPool(ctx->device, &desc_pool_info, NULL, &temp_pool) != VK_SUCCESS) {
        goto cleanup_c;
    }

    VkDescriptorSetLayout *layouts = (VkDescriptorSetLayout *)calloc(
        (size_t)batch_size, sizeof(VkDescriptorSetLayout));
    VkDescriptorSet *desc_sets = (VkDescriptorSet *)calloc(
        (size_t)batch_size, sizeof(VkDescriptorSet));
    if (!layouts || !desc_sets) {
        free(layouts);
        free(desc_sets);
        goto cleanup_pool;
    }
    for (int b = 0; b < batch_size; ++b) layouts[b] = ctx->descriptor_layout;

    VkDescriptorSetAllocateInfo set_alloc;
    memset(&set_alloc, 0, sizeof(set_alloc));
    set_alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    set_alloc.descriptorPool = temp_pool;
    set_alloc.descriptorSetCount = (uint32_t)batch_size;
    set_alloc.pSetLayouts = layouts;
    if (ctx->dev.vkAllocateDescriptorSets(ctx->device, &set_alloc, desc_sets) != VK_SUCCESS) {
        free(layouts);
        free(desc_sets);
        goto cleanup_pool;
    }
    free(layouts);
    layouts = NULL;

    for (int b = 0; b < batch_size; ++b) {
        VkDescriptorBufferInfo buf_infos[3];
        memset(buf_infos, 0, sizeof(buf_infos));
        buf_infos[0].buffer = buf_a.buffer; buf_infos[0].offset = 0; buf_infos[0].range = a_bytes;
        buf_infos[1].buffer = buf_b.buffer; buf_infos[1].offset = 0; buf_infos[1].range = b_bytes;
        buf_infos[2].buffer = c_bufs[b].buffer; buf_infos[2].offset = 0; buf_infos[2].range = c_bytes;
        VkWriteDescriptorSet writes[3];
        memset(writes, 0, sizeof(writes));
        for (uint32_t i = 0; i < 3; ++i) {
            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = desc_sets[b];
            writes[i].dstBinding = i;
            writes[i].descriptorCount = 1;
            writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writes[i].pBufferInfo = &buf_infos[i];
        }
        ctx->dev.vkUpdateDescriptorSets(ctx->device, 3, writes, 0, NULL);
    }

    /* Allocate N command buffers from the existing (resettable) pool. */
    VkCommandBuffer *cmd_bufs = (VkCommandBuffer *)calloc(
        (size_t)batch_size, sizeof(VkCommandBuffer));
    if (!cmd_bufs) {
        free(desc_sets);
        goto cleanup_pool;
    }
    VkCommandBufferAllocateInfo cmd_alloc;
    memset(&cmd_alloc, 0, sizeof(cmd_alloc));
    cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc.commandPool = ctx->command_pool;
    cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_alloc.commandBufferCount = (uint32_t)batch_size;
    if (ctx->dev.vkAllocateCommandBuffers(ctx->device, &cmd_alloc, cmd_bufs) != VK_SUCCESS) {
        free(cmd_bufs);
        free(desc_sets);
        goto cleanup_pool;
    }

    const uint32_t group_x = ((uint32_t)out_cols + 15u) / 16u;
    const uint32_t group_y = ((uint32_t)rows + 15u) / 16u;
    StrixPushConstants push;
    push.rows  = (uint32_t)rows;
    push.cols  = (uint32_t)out_cols;
    push.inner = (uint32_t)inner_dim;

    VkMemoryBarrier barrier;
    memset(&barrier, 0, sizeof(barrier));
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;

    int record_ok = 1;
    for (int b = 0; b < batch_size; ++b) {
        VkCommandBufferBeginInfo begin;
        memset(&begin, 0, sizeof(begin));
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (ctx->dev.vkBeginCommandBuffer(cmd_bufs[b], &begin) != VK_SUCCESS) {
            record_ok = 0; break;
        }
        ctx->dev.vkCmdBindPipeline(cmd_bufs[b], VK_PIPELINE_BIND_POINT_COMPUTE, ctx->pipeline);
        ctx->dev.vkCmdBindDescriptorSets(cmd_bufs[b], VK_PIPELINE_BIND_POINT_COMPUTE,
                                         ctx->pipeline_layout, 0, 1, &desc_sets[b], 0, NULL);
        ctx->dev.vkCmdPushConstants(cmd_bufs[b], ctx->pipeline_layout,
                                    VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
        ctx->dev.vkCmdDispatch(cmd_bufs[b], group_x, group_y, 1);
        ctx->dev.vkCmdPipelineBarrier(cmd_bufs[b],
                                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                      VK_PIPELINE_STAGE_HOST_BIT,
                                      0, 1, &barrier, 0, NULL, 0, NULL);
        if (ctx->dev.vkEndCommandBuffer(cmd_bufs[b]) != VK_SUCCESS) {
            record_ok = 0; break;
        }
    }

    if (!record_ok) goto cleanup_cmds;

    /* Single vkQueueSubmit() for the entire batch. */
    VkSubmitInfo *submits = (VkSubmitInfo *)calloc((size_t)batch_size, sizeof(VkSubmitInfo));
    if (!submits) goto cleanup_cmds;
    for (int b = 0; b < batch_size; ++b) {
        submits[b].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submits[b].commandBufferCount = 1;
        submits[b].pCommandBuffers = &cmd_bufs[b];
    }
    if (ctx->dev.vkResetFences(ctx->device, 1, &ctx->fence) != VK_SUCCESS) {
        free(submits);
        goto cleanup_cmds;
    }
    VkResult submit_res = ctx->dev.vkQueueSubmit(ctx->queue, (uint32_t)batch_size,
                                                  submits, ctx->fence);
    free(submits);
    if (submit_res != VK_SUCCESS) {
        vk_debugf("vkQueueSubmit (batch %d) failed", batch_size);
        goto cleanup_cmds;
    }

    /* One fence wait covers all submitted command buffers. */
    if (ctx->dev.vkWaitForFences(ctx->device, 1, &ctx->fence, VK_TRUE,
                                  10ull * 1000ull * 1000ull * 1000ull) != VK_SUCCESS) {
        vk_debugf("vkWaitForFences (batch %d) timed out or failed", batch_size);
        goto cleanup_cmds;
    }

    /* Read back and (optionally) scale all outputs. */
    ok = 1;
    for (int b = 0; b < batch_size; ++b) {
        if (ctx->dev.vkMapMemory(ctx->device, c_bufs[b].memory, 0, c_bytes, 0, &mapped) != VK_SUCCESS) {
            ok = 0;
            break;
        }
        float *dst = output + (size_t)b * c_count;
        memcpy(dst, mapped, (size_t)c_bytes);
        ctx->dev.vkUnmapMemory(ctx->device, c_bufs[b].memory);
        if (scales) {
            for (int r = 0; r < rows; ++r)
                for (int o = 0; o < out_cols; ++o)
                    dst[(size_t)r * (size_t)out_cols + (size_t)o] *= scales[o];
        }
    }

cleanup_cmds:
    ctx->dev.vkFreeCommandBuffers(ctx->device, ctx->command_pool,
                                   (uint32_t)batch_size, cmd_bufs);
    free(cmd_bufs);
    free(desc_sets);
cleanup_pool:
    if (temp_pool != VK_NULL_HANDLE)
        ctx->dev.vkDestroyDescriptorPool(ctx->device, temp_pool, NULL);
cleanup_c:
    for (int b = 0; b < c_count_alloc; ++b) destroy_buffer(ctx, &c_bufs[b]);
    free(c_bufs);
cleanup_ab:
    destroy_buffer(ctx, &buf_b);
    destroy_buffer(ctx, &buf_a);
    return ok;
}

int strix_vulkan_batch_matmul(const int8_t *input,
                              int rows,
                              int inner_dim,
                              const int8_t *weights,
                              int out_cols,
                              float *output,
                              const float *scales,
                              int batch_size) {
    if (!input || !weights || !output || rows <= 0 || inner_dim <= 0 || out_cols <= 0) {
        return 0;
    }
    if (batch_size <= 0 || batch_size > STRIX_VULKAN_MAX_BATCH) {
        vk_debugf("batch_size %d out of range [1, %d]", batch_size, STRIX_VULKAN_MAX_BATCH);
        return 0;
    }
    if (!ensure_context()) return 0;
    return run_batch_matmul(&g_ctx, input, rows, inner_dim, weights, out_cols,
                            output, scales, batch_size);
}

const char *strix_vulkan_backend_name(void) {
    return ensure_context() ? "vulkan-compute-strix-halo" : "vulkan-unavailable";
}

const char *strix_vulkan_device_name(void) {
    return ensure_context() ? g_ctx.device_name : "";
}

size_t strix_vulkan_resident_bytes(void) {
    if (!ensure_context()) return 0;

    VkPhysicalDeviceMemoryProperties props;
    memset(&props, 0, sizeof(props));
    g_ctx.vk.vkGetPhysicalDeviceMemoryProperties(g_ctx.physical_device, &props);

    /*
     * The largest DEVICE_LOCAL heap. On Strix Halo that heap is system memory,
     * so this is a genuine upper bound on what one dispatch can hold — not the
     * discrete-GPU sense of "VRAM". Heaps with no DEVICE_LOCAL bit are ignored:
     * an allocation there is not somewhere the shader reads from cheaply, and
     * counting it would inflate the budget placement is checking against.
     */
    VkDeviceSize best = 0;
    for (uint32_t i = 0; i < props.memoryHeapCount; ++i) {
        if (!(props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)) continue;
        if (props.memoryHeaps[i].size > best) best = props.memoryHeaps[i].size;
    }
    if (best == 0 || (uint64_t)best > (uint64_t)SIZE_MAX) return 0;
    return (size_t)best;
}

void strix_vulkan_shutdown(void) {
    if (g_init_attempted) {
        destroy_context(&g_ctx);
        g_init_attempted = 0;
        g_failure_reason = "not initialised";
    }
}
