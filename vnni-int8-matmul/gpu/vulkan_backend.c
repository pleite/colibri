#include "vulkan_backend.h"

#include <dlfcn.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cpu/vnni_cpu_backend.h"
#include "vulkan/vulkan.h"

typedef struct {
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physical_device;
    VkQueue queue;
    uint32_t queue_family_index;
    VkCommandPool command_pool;
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    VkShaderModule shader_module;
    VkDescriptorSetLayout descriptor_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set;
    VkBuffer buffers[3];
    VkDeviceMemory memories[3];
    VkCommandBuffer command_buffer;
    VnnVulkanDispatch dispatch;
    bool initialized;
} StrixVulkanContext;

typedef struct {
    int rows;
    int cols;
    int inner;
} StrixPushConstants;

static void *g_vulkan_handle = NULL;
static int g_vulkan_available = 0;
static int g_vulkan_initialized = 0;
static VnnVulkanDispatch g_vulkan_dispatch;

static void vulkan_debugf(const char *fmt, ...) {
    const char *enabled = getenv("VNNI_VULKAN_DEBUG");
    if (!enabled || !enabled[0] || strcmp(enabled, "0") == 0) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "[vulkan-debug] ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

static void release_weights(float *weights_f32, int fmt) {
    if (fmt != 0) {
        free(weights_f32);
    }
}

static int read_shader_binary(const char *path, uint32_t **code, size_t *code_size) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0; }
    long size = ftell(fp);
    if (size < 0) { fclose(fp); return 0; }
    rewind(fp);
    uint32_t *buffer = (uint32_t *)malloc((size_t)size);
    if (!buffer) { fclose(fp); return 0; }
    if (fread(buffer, 1, (size_t)size, fp) != (size_t)size) { free(buffer); fclose(fp); return 0; }
    fclose(fp);
    *code = buffer;
    *code_size = (size_t)size;
    return 1;
}

static const char *shader_path(void) {
    return "gpu/comp.spv";
}

static uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t filter, uint32_t flags, VnnVulkanDispatch *dispatch) {
    VkPhysicalDeviceMemoryProperties props;
    dispatch->vkGetPhysicalDeviceMemoryProperties(physical_device, &props);
    for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
        if ((filter & (1u << i)) && (props.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }
    return UINT32_MAX;
}

static void destroy_context(StrixVulkanContext *ctx) {
    if (!ctx) return;
    if (ctx->device) {
        if (ctx->pipeline && ctx->dispatch.vkDestroyPipeline) {
            ctx->dispatch.vkDestroyPipeline(ctx->device, ctx->pipeline, NULL);
        }
        if (ctx->pipeline_layout && ctx->dispatch.vkDestroyPipelineLayout) {
            ctx->dispatch.vkDestroyPipelineLayout(ctx->device, ctx->pipeline_layout, NULL);
        }
        if (ctx->shader_module && ctx->dispatch.vkDestroyShaderModule) {
            ctx->dispatch.vkDestroyShaderModule(ctx->device, ctx->shader_module, NULL);
        }
        if (ctx->descriptor_layout && ctx->dispatch.vkDestroyDescriptorSetLayout) {
            ctx->dispatch.vkDestroyDescriptorSetLayout(ctx->device, ctx->descriptor_layout, NULL);
        }
        if (ctx->descriptor_pool && ctx->dispatch.vkDestroyDescriptorPool) {
            ctx->dispatch.vkDestroyDescriptorPool(ctx->device, ctx->descriptor_pool, NULL);
        }
        if (ctx->command_pool && ctx->dispatch.vkDestroyCommandPool) {
            ctx->dispatch.vkDestroyCommandPool(ctx->device, ctx->command_pool, NULL);
        }
        for (int i = 0; i < 3; ++i) {
            if (ctx->buffers[i] && ctx->dispatch.vkDestroyBuffer) {
                ctx->dispatch.vkDestroyBuffer(ctx->device, ctx->buffers[i], NULL);
            }
            if (ctx->memories[i] && ctx->dispatch.vkFreeMemory) {
                ctx->dispatch.vkFreeMemory(ctx->device, ctx->memories[i], NULL);
            }
        }
        if (ctx->dispatch.vkDestroyDevice) {
            ctx->dispatch.vkDestroyDevice(ctx->device, NULL);
        }
    }
    if (ctx->instance && ctx->dispatch.vkDestroyInstance) {
        ctx->dispatch.vkDestroyInstance(ctx->instance, NULL);
    }
    memset(ctx, 0, sizeof(*ctx));
}

static int load_dispatch(VnnVulkanDispatch *dispatch) {
    memset(dispatch, 0, sizeof(*dispatch));
    const char *candidates[] = {"libvulkan.so.1", "libvulkan.so"};
    const char *loaded_name = NULL;
    const char *load_error = "unknown";

    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        dlerror();
        void *handle = dlopen(candidates[i], RTLD_NOW | RTLD_LOCAL);
        if (handle) {
            g_vulkan_handle = handle;
            loaded_name = candidates[i];
            break;
        }
        load_error = dlerror();
        vulkan_debugf("dlopen(%s) failed: %s", candidates[i], load_error ? load_error : "unknown");
    }
    if (!g_vulkan_handle) {
        vulkan_debugf("failed to load Vulkan loader: %s", load_error ? load_error : "unknown");
        return 0;
    }

    PFN_vkGetInstanceProcAddr get_proc_addr = (PFN_vkGetInstanceProcAddr)dlsym(g_vulkan_handle, "vkGetInstanceProcAddr");
    if (!get_proc_addr) {
        vulkan_debugf("vkGetInstanceProcAddr not found");
        dlclose(g_vulkan_handle);
        g_vulkan_handle = NULL;
        return 0;
    }

    dispatch->vkCreateInstance = (PFN_vkCreateInstance)dlsym(g_vulkan_handle, "vkCreateInstance");
    dispatch->vkDestroyInstance = (PFN_vkDestroyInstance)dlsym(g_vulkan_handle, "vkDestroyInstance");
    dispatch->vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)dlsym(g_vulkan_handle, "vkEnumeratePhysicalDevices");
    dispatch->vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)dlsym(g_vulkan_handle, "vkGetPhysicalDeviceProperties");
    dispatch->vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)dlsym(g_vulkan_handle, "vkGetPhysicalDeviceQueueFamilyProperties");
    dispatch->vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)dlsym(g_vulkan_handle, "vkGetPhysicalDeviceMemoryProperties");
    dispatch->vkCreateDevice = (PFN_vkCreateDevice)dlsym(g_vulkan_handle, "vkCreateDevice");
    dispatch->vkDestroyDevice = (PFN_vkDestroyDevice)dlsym(g_vulkan_handle, "vkDestroyDevice");

    vulkan_debugf("loaded Vulkan dispatcher from %s", loaded_name ? loaded_name : "unknown");
    return dispatch->vkCreateInstance && dispatch->vkDestroyInstance && dispatch->vkEnumeratePhysicalDevices && dispatch->vkGetPhysicalDeviceProperties && dispatch->vkGetPhysicalDeviceQueueFamilyProperties && dispatch->vkGetPhysicalDeviceMemoryProperties && dispatch->vkCreateDevice && dispatch->vkDestroyDevice;
}

static int resolve_device_dispatch(StrixVulkanContext *ctx) {
    if (!g_vulkan_handle || !ctx || !ctx->instance || !ctx->device) {
        return 0;
    }
    PFN_vkGetInstanceProcAddr get_proc_addr = (PFN_vkGetInstanceProcAddr)dlsym(g_vulkan_handle, "vkGetInstanceProcAddr");
    if (!get_proc_addr) {
        vulkan_debugf("vkGetInstanceProcAddr unavailable while resolving device functions");
        return 0;
    }
    PFN_vkGetDeviceProcAddr get_device_proc_addr = (PFN_vkGetDeviceProcAddr)get_proc_addr(ctx->instance, "vkGetDeviceProcAddr");
    if (!get_device_proc_addr) {
        vulkan_debugf("vkGetDeviceProcAddr unavailable while resolving device functions");
        return 0;
    }

#define RESOLVE_DEVICE_PROC(name)                                              \
    do {                                                                       \
        ctx->dispatch.name = (__typeof__(ctx->dispatch.name))get_device_proc_addr(ctx->device, #name); \
        if (!ctx->dispatch.name) {                                              \
            vulkan_debugf("failed to resolve device entry point %s", #name);   \
            return 0;                                                           \
        }                                                                       \
    } while (0)

    RESOLVE_DEVICE_PROC(vkGetDeviceQueue);
    RESOLVE_DEVICE_PROC(vkCreateCommandPool);
    RESOLVE_DEVICE_PROC(vkDestroyCommandPool);
    RESOLVE_DEVICE_PROC(vkAllocateCommandBuffers);
    RESOLVE_DEVICE_PROC(vkFreeCommandBuffers);
    RESOLVE_DEVICE_PROC(vkBeginCommandBuffer);
    RESOLVE_DEVICE_PROC(vkEndCommandBuffer);
    RESOLVE_DEVICE_PROC(vkCreateDescriptorSetLayout);
    RESOLVE_DEVICE_PROC(vkDestroyDescriptorSetLayout);
    RESOLVE_DEVICE_PROC(vkCreateShaderModule);
    RESOLVE_DEVICE_PROC(vkDestroyShaderModule);
    RESOLVE_DEVICE_PROC(vkCreatePipelineLayout);
    RESOLVE_DEVICE_PROC(vkDestroyPipelineLayout);
    RESOLVE_DEVICE_PROC(vkCreateComputePipelines);
    RESOLVE_DEVICE_PROC(vkDestroyPipeline);
    RESOLVE_DEVICE_PROC(vkCreateDescriptorPool);
    RESOLVE_DEVICE_PROC(vkDestroyDescriptorPool);
    RESOLVE_DEVICE_PROC(vkAllocateDescriptorSets);
    RESOLVE_DEVICE_PROC(vkUpdateDescriptorSets);
    RESOLVE_DEVICE_PROC(vkCreateBuffer);
    RESOLVE_DEVICE_PROC(vkDestroyBuffer);
    RESOLVE_DEVICE_PROC(vkGetBufferMemoryRequirements);
    RESOLVE_DEVICE_PROC(vkAllocateMemory);
    RESOLVE_DEVICE_PROC(vkFreeMemory);
    RESOLVE_DEVICE_PROC(vkBindBufferMemory);
    RESOLVE_DEVICE_PROC(vkMapMemory);
    RESOLVE_DEVICE_PROC(vkUnmapMemory);
    RESOLVE_DEVICE_PROC(vkCmdBindPipeline);
    RESOLVE_DEVICE_PROC(vkCmdBindDescriptorSets);
    RESOLVE_DEVICE_PROC(vkCmdPushConstants);
    RESOLVE_DEVICE_PROC(vkCmdDispatch);
    RESOLVE_DEVICE_PROC(vkQueueSubmit);
    RESOLVE_DEVICE_PROC(vkWaitForFences);
    RESOLVE_DEVICE_PROC(vkCreateFence);
    RESOLVE_DEVICE_PROC(vkDestroyFence);

#undef RESOLVE_DEVICE_PROC
    return 1;
}

static int create_context(StrixVulkanContext *ctx, VnnVulkanDispatch *dispatch) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->dispatch = *dispatch;

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "colibri-vnni-vulkan";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "colibri";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo instance_info = {0};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    if (dispatch->vkCreateInstance(&instance_info, NULL, &ctx->instance) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkCreateInstance failed");
        return 0;
    }

    uint32_t device_count = 0;
    if (dispatch->vkEnumeratePhysicalDevices(ctx->instance, &device_count, NULL) != VK_SUCCESS || device_count == 0) {
        vulkan_debugf("create_context: vkEnumeratePhysicalDevices returned %u devices", device_count);
        destroy_context(ctx);
        return 0;
    }
    VkPhysicalDevice devices[8] = {0};
    if (dispatch->vkEnumeratePhysicalDevices(ctx->instance, &device_count, devices) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkEnumeratePhysicalDevices failed during device enumeration");
        destroy_context(ctx);
        return 0;
    }

    for (uint32_t i = 0; i < device_count; ++i) {
        VkPhysicalDeviceProperties props = {0};
        dispatch->vkGetPhysicalDeviceProperties(devices[i], &props);
        uint32_t family_count = 0;
        dispatch->vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &family_count, NULL);
        VkQueueFamilyProperties families[8] = {0};
        if (family_count > 8) family_count = 8;
        dispatch->vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &family_count, families);
        for (uint32_t j = 0; j < family_count; ++j) {
            if (families[j].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                ctx->physical_device = devices[i];
                ctx->queue_family_index = j;
                goto found_device;
            }
        }
    }
    destroy_context(ctx);
    return 0;
found_device:

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = {0};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = ctx->queue_family_index;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;

    VkDeviceCreateInfo device_info = {0};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    if (dispatch->vkCreateDevice(ctx->physical_device, &device_info, NULL, &ctx->device) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkCreateDevice failed");
        destroy_context(ctx);
        return 0;
    }
    if (!resolve_device_dispatch(ctx)) {
        vulkan_debugf("create_context: failed to resolve device entry points");
        destroy_context(ctx);
        return 0;
    }
    ctx->dispatch.vkGetDeviceQueue(ctx->device, ctx->queue_family_index, 0, &ctx->queue);

    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = ctx->queue_family_index;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (ctx->dispatch.vkCreateCommandPool(ctx->device, &pool_info, NULL, &ctx->command_pool) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkCreateCommandPool failed");
        destroy_context(ctx);
        return 0;
    }

    VkDescriptorSetLayoutBinding bindings[3] = {0};
    for (int i = 0; i < 3; ++i) {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    VkDescriptorSetLayoutCreateInfo layout_info = {0};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 3;
    layout_info.pBindings = bindings;
    if (ctx->dispatch.vkCreateDescriptorSetLayout(ctx->device, &layout_info, NULL, &ctx->descriptor_layout) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkCreateDescriptorSetLayout failed");
        destroy_context(ctx);
        return 0;
    }

    size_t shader_size = 0;
    uint32_t *shader_code = NULL;
    if (!read_shader_binary(shader_path(), &shader_code, &shader_size)) {
        destroy_context(ctx);
        return 0;
    }
    VkShaderModuleCreateInfo shader_info = {0};
    shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_info.codeSize = shader_size;
    shader_info.pCode = shader_code;
    if (ctx->dispatch.vkCreateShaderModule(ctx->device, &shader_info, NULL, &ctx->shader_module) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkCreateShaderModule failed");
        free(shader_code);
        destroy_context(ctx);
        return 0;
    }
    free(shader_code);

    VkPushConstantRange push_range = {0};
    push_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_range.offset = 0;
    push_range.size = sizeof(StrixPushConstants);

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &ctx->descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_range;
    if (ctx->dispatch.vkCreatePipelineLayout(ctx->device, &pipeline_layout_info, NULL, &ctx->pipeline_layout) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkCreatePipelineLayout failed");
        destroy_context(ctx);
        return 0;
    }

    VkPipelineShaderStageCreateInfo stage = {0};
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = ctx->shader_module;
    stage.pName = "main";

    VkComputePipelineCreateInfo pipeline_info = {0};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.stage = stage;
    pipeline_info.layout = ctx->pipeline_layout;
    if (ctx->dispatch.vkCreateComputePipelines(ctx->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &ctx->pipeline) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkCreateComputePipelines failed");
        destroy_context(ctx);
        return 0;
    }

    VkDescriptorPoolSize pool_sizes[1] = {0};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_sizes[0].descriptorCount = 3;
    VkDescriptorPoolCreateInfo pool_info2 = {0};
    pool_info2.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info2.maxSets = 1;
    pool_info2.poolSizeCount = 1;
    pool_info2.pPoolSizes = pool_sizes;
    if (ctx->dispatch.vkCreateDescriptorPool(ctx->device, &pool_info2, NULL, &ctx->descriptor_pool) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkCreateDescriptorPool failed");
        destroy_context(ctx);
        return 0;
    }

    VkDescriptorSetAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = ctx->descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &ctx->descriptor_layout;
    if (ctx->dispatch.vkAllocateDescriptorSets(ctx->device, &alloc_info, &ctx->descriptor_set) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkAllocateDescriptorSets failed");
        destroy_context(ctx);
        return 0;
    }

    VkCommandBufferAllocateInfo cmd_alloc = {0};
    cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc.commandPool = ctx->command_pool;
    cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_alloc.commandBufferCount = 1;
    if (ctx->dispatch.vkAllocateCommandBuffers(ctx->device, &cmd_alloc, &ctx->command_buffer) != VK_SUCCESS) {
        vulkan_debugf("create_context: vkAllocateCommandBuffers failed");
        destroy_context(ctx);
        return 0;
    }

    ctx->initialized = true;
    return 1;
}

static int create_buffer(StrixVulkanContext *ctx, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory) {
    VkBufferCreateInfo info = {0};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (ctx->dispatch.vkCreateBuffer(ctx->device, &info, NULL, buffer) != VK_SUCCESS) {
        vulkan_debugf("create_buffer: vkCreateBuffer failed for size=%llu", (unsigned long long)size);
        return 0;
    }

    VkMemoryRequirements requirements = {0};
    ctx->dispatch.vkGetBufferMemoryRequirements(ctx->device, *buffer, &requirements);
    uint32_t index = find_memory_type(ctx->physical_device, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &ctx->dispatch);
    if (index == UINT32_MAX) {
        vulkan_debugf("create_buffer: no suitable memory type found for %llu bytes", (unsigned long long)size);
        ctx->dispatch.vkDestroyBuffer(ctx->device, *buffer, NULL);
        return 0;
    }
    VkMemoryAllocateInfo alloc = {0};
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.allocationSize = requirements.size;
    alloc.memoryTypeIndex = index;
    if (ctx->dispatch.vkAllocateMemory(ctx->device, &alloc, NULL, memory) != VK_SUCCESS) {
        vulkan_debugf("create_buffer: vkAllocateMemory failed");
        ctx->dispatch.vkDestroyBuffer(ctx->device, *buffer, NULL);
        return 0;
    }
    if (ctx->dispatch.vkBindBufferMemory(ctx->device, *buffer, *memory, 0) != VK_SUCCESS) {
        vulkan_debugf("create_buffer: vkBindBufferMemory failed");
        ctx->dispatch.vkFreeMemory(ctx->device, *memory, NULL);
        ctx->dispatch.vkDestroyBuffer(ctx->device, *buffer, NULL);
        return 0;
    }
    return 1;
}

static int run_vulkan_matmul(StrixVulkanContext *ctx, const int8_t *input, const void *weights, const float *scales, int fmt, int S, int I, int O, float *y) {
    if (!ctx->initialized) return 0;
    if (fmt != 0 && fmt != 1) return 0;

    const size_t a_bytes = (size_t)S * (size_t)I * sizeof(float);
    const size_t b_bytes = (size_t)I * (size_t)O * sizeof(float);
    const size_t c_bytes = (size_t)S * (size_t)O * sizeof(float);

    float *weights_f32 = NULL;
    if (fmt == 0) {
        weights_f32 = (float *)weights;
    } else {
        const int8_t *weights_i8 = (const int8_t *)weights;
        weights_f32 = (float *)calloc((size_t)O * (size_t)I, sizeof(float));
        if (!weights_f32) return 0;
        for (int o = 0; o < O; ++o) {
            for (int i = 0; i < I; ++i) {
                weights_f32[(size_t)o * (size_t)I + i] = (float)weights_i8[(size_t)o * (size_t)I + i];
            }
        }
    }

    float *weights_t = (float *)calloc((size_t)I * (size_t)O, sizeof(float));
    if (!weights_t) { release_weights(weights_f32, fmt); return 0; }

    float *input_f32 = (float *)calloc((size_t)S * (size_t)I, sizeof(float));
    if (!input_f32) { free(weights_t); release_weights(weights_f32, fmt); return 0; }
    for (int s = 0; s < S; ++s) {
        for (int i = 0; i < I; ++i) {
            input_f32[(size_t)s * (size_t)I + i] = (float)input[(size_t)s * (size_t)I + i];
        }
    }
    for (int o = 0; o < O; ++o) {
        for (int i = 0; i < I; ++i) {
            weights_t[(size_t)i * (size_t)O + o] = weights_f32[(size_t)o * (size_t)I + i];
        }
    }

    if (!create_buffer(ctx, a_bytes, &ctx->buffers[0], &ctx->memories[0])) {
        vulkan_debugf("run_vulkan_matmul: failed to create input buffer (%llu bytes)", (unsigned long long)a_bytes);
        free(weights_t); release_weights(weights_f32, fmt); return 0;
    }
    if (!create_buffer(ctx, b_bytes, &ctx->buffers[1], &ctx->memories[1])) {
        vulkan_debugf("run_vulkan_matmul: failed to create weight buffer (%llu bytes)", (unsigned long long)b_bytes);
        free(weights_t); release_weights(weights_f32, fmt); return 0;
    }
    if (!create_buffer(ctx, c_bytes, &ctx->buffers[2], &ctx->memories[2])) {
        vulkan_debugf("run_vulkan_matmul: failed to create output buffer (%llu bytes)", (unsigned long long)c_bytes);
        free(weights_t); release_weights(weights_f32, fmt); return 0;
    }

    void *mapped_a = NULL;
    void *mapped_b = NULL;
    void *mapped_c = NULL;
    if (ctx->dispatch.vkMapMemory(ctx->device, ctx->memories[0], 0, a_bytes, 0, &mapped_a) != VK_SUCCESS ||
        ctx->dispatch.vkMapMemory(ctx->device, ctx->memories[1], 0, b_bytes, 0, &mapped_b) != VK_SUCCESS ||
        ctx->dispatch.vkMapMemory(ctx->device, ctx->memories[2], 0, c_bytes, 0, &mapped_c) != VK_SUCCESS) {
        vulkan_debugf("run_vulkan_matmul: vkMapMemory failed while preparing buffers");
        ctx->dispatch.vkUnmapMemory(ctx->device, ctx->memories[0]);
        ctx->dispatch.vkUnmapMemory(ctx->device, ctx->memories[1]);
        ctx->dispatch.vkUnmapMemory(ctx->device, ctx->memories[2]);
        free(input_f32); free(weights_t); release_weights(weights_f32, fmt); return 0;
    }
    memcpy(mapped_a, input_f32, a_bytes);
    memcpy(mapped_b, weights_t, b_bytes);
    memset(mapped_c, 0, c_bytes);
    ctx->dispatch.vkUnmapMemory(ctx->device, ctx->memories[0]);
    ctx->dispatch.vkUnmapMemory(ctx->device, ctx->memories[1]);
    ctx->dispatch.vkUnmapMemory(ctx->device, ctx->memories[2]);

    VkDescriptorBufferInfo infos[3] = {0};
    infos[0].buffer = ctx->buffers[0]; infos[0].offset = 0; infos[0].range = a_bytes;
    infos[1].buffer = ctx->buffers[1]; infos[1].offset = 0; infos[1].range = b_bytes;
    infos[2].buffer = ctx->buffers[2]; infos[2].offset = 0; infos[2].range = c_bytes;
    VkWriteDescriptorSet writes[3] = {0};
    for (int i = 0; i < 3; ++i) {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = ctx->descriptor_set;
        writes[i].dstBinding = i;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[i].descriptorCount = 1;
        writes[i].pBufferInfo = &infos[i];
    }
    ctx->dispatch.vkUpdateDescriptorSets(ctx->device, 3, writes, 0, NULL);

    VkCommandBufferBeginInfo begin = {0};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (ctx->dispatch.vkBeginCommandBuffer(ctx->command_buffer, &begin) != VK_SUCCESS) {
        vulkan_debugf("run_vulkan_matmul: vkBeginCommandBuffer failed");
        free(input_f32); free(weights_t); release_weights(weights_f32, fmt); return 0;
    }
    ctx->dispatch.vkCmdBindPipeline(ctx->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, ctx->pipeline);
    ctx->dispatch.vkCmdBindDescriptorSets(ctx->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, ctx->pipeline_layout, 0, 1, &ctx->descriptor_set, 0, NULL);
    StrixPushConstants push = {S, O, I};
    ctx->dispatch.vkCmdPushConstants(ctx->command_buffer, ctx->pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
    const uint32_t group_x = (O + 15) / 16;
    const uint32_t group_y = (S + 15) / 16;
    ctx->dispatch.vkCmdDispatch(ctx->command_buffer, group_x, group_y, 1);
    if (ctx->dispatch.vkEndCommandBuffer(ctx->command_buffer) != VK_SUCCESS) {
        vulkan_debugf("run_vulkan_matmul: vkEndCommandBuffer failed");
        free(input_f32); free(weights_t); release_weights(weights_f32, fmt); return 0;
    }

    VkSubmitInfo submit = {0};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &ctx->command_buffer;
    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (ctx->dispatch.vkCreateFence(ctx->device, &fence_info, NULL, &fence) != VK_SUCCESS) {
        vulkan_debugf("run_vulkan_matmul: vkCreateFence failed");
        free(input_f32); free(weights_t); release_weights(weights_f32, fmt); return 0;
    }
    int ok = (ctx->dispatch.vkQueueSubmit(ctx->queue, 1, &submit, fence) == VK_SUCCESS && ctx->dispatch.vkWaitForFences(ctx->device, 1, &fence, VK_TRUE, UINT64_MAX) == VK_SUCCESS);
    ctx->dispatch.vkDestroyFence(ctx->device, fence, NULL);
    if (!ok) {
        vulkan_debugf("run_vulkan_matmul: queue submission or fence wait failed");
        free(input_f32); free(weights_t); release_weights(weights_f32, fmt); return 0;
    }

    void *mapped_out = NULL;
    if (ctx->dispatch.vkMapMemory(ctx->device, ctx->memories[2], 0, c_bytes, 0, &mapped_out) != VK_SUCCESS) {
        free(input_f32); free(weights_t); release_weights(weights_f32, fmt); return 0;
    }
    memcpy(y, mapped_out, c_bytes);
    ctx->dispatch.vkUnmapMemory(ctx->device, ctx->memories[2]);

    if (scales && fmt != 0) {
        for (int s = 0; s < S; ++s) {
            for (int o = 0; o < O; ++o) {
                y[(size_t)s * (size_t)O + o] *= scales[o];
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        if (ctx->buffers[i]) ctx->dispatch.vkDestroyBuffer(ctx->device, ctx->buffers[i], NULL);
        if (ctx->memories[i]) ctx->dispatch.vkFreeMemory(ctx->device, ctx->memories[i], NULL);
        ctx->buffers[i] = VK_NULL_HANDLE;
        ctx->memories[i] = VK_NULL_HANDLE;
    }
    free(input_f32);
    free(weights_t);
    release_weights(weights_f32, fmt);
    return 1;
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

    if (!g_vulkan_initialized) {
        if (!load_dispatch(&g_vulkan_dispatch)) {
            vulkan_debugf("strix_vulkan_matmul: Vulkan loader unavailable");
            g_vulkan_available = 0;
            g_vulkan_initialized = 1;
            return 0;
        }
        g_vulkan_available = 1;
        g_vulkan_initialized = 1;
    }

    if (!g_vulkan_available) {
        return 0;
    }

    StrixVulkanContext ctx;
    if (!create_context(&ctx, &g_vulkan_dispatch)) {
        vulkan_debugf("strix_vulkan_matmul: Vulkan context creation failed");
        g_vulkan_available = 0;
        return 0;
    }

    if (!run_vulkan_matmul(&ctx, input, weights, scales, 1, rows, inner_dim, out_cols, output)) {
        vulkan_debugf("strix_vulkan_matmul: Vulkan execution failed");
        g_vulkan_available = 0;
        destroy_context(&ctx);
        return 0;
    }
    destroy_context(&ctx);
    return 1;
}

const char *strix_vulkan_backend_name(void) {
    return g_vulkan_available ? "vulkan-compute" : "vulkan-unavailable";
}
