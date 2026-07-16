#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

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
    bool initialized;
} ColiVulkanNativeContext;

typedef struct {
    int rows;
    int cols;
    int inner;
} ColiPushConstants;

static void cpu_matmul(float *y, const float *x, const void *weights, const float *scales, int fmt, int S, int I, int O) {
    const float *weights_f32 = (const float *)weights;
    const int8_t *weights_i8 = (const int8_t *)weights;
    for (int s = 0; s < S; ++s) {
        for (int o = 0; o < O; ++o) {
            float acc = 0.0f;
            for (int i = 0; i < I; ++i) {
                if (fmt == 0) {
                    acc += x[(size_t)s * (size_t)I + i] * weights_f32[(size_t)o * (size_t)I + i];
                } else {
                    acc += x[(size_t)s * (size_t)I + i] * (float)weights_i8[(size_t)o * (size_t)I + i];
                }
            }
            y[(size_t)s * (size_t)O + o] = acc * (scales && fmt != 0 ? scales[o] : 1.0f);
        }
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
    const char *from_env = getenv("COLI_VULKAN_SHADER_PATH");
    if (from_env && *from_env) return from_env;
    return "tests/shaders/comp.spv";
}

static uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t filter, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &props);
    for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
        if ((filter & (1u << i)) && (props.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }
    return UINT32_MAX;
}

static void destroy_context(ColiVulkanNativeContext *ctx) {
    if (!ctx) return;
    if (ctx->descriptor_set) {
        /* no-op, set is owned by descriptor pool */
    }
    if (ctx->command_buffer) {
        /* no-op, command buffer is allocated from pool */
    }
    if (ctx->device) {
        if (ctx->pipeline) vkDestroyPipeline(ctx->device, ctx->pipeline, NULL);
        if (ctx->pipeline_layout) vkDestroyPipelineLayout(ctx->device, ctx->pipeline_layout, NULL);
        if (ctx->shader_module) vkDestroyShaderModule(ctx->device, ctx->shader_module, NULL);
        if (ctx->descriptor_layout) vkDestroyDescriptorSetLayout(ctx->device, ctx->descriptor_layout, NULL);
        if (ctx->descriptor_pool) vkDestroyDescriptorPool(ctx->device, ctx->descriptor_pool, NULL);
        if (ctx->command_pool) vkDestroyCommandPool(ctx->device, ctx->command_pool, NULL);
        for (int i = 0; i < 3; ++i) {
            if (ctx->buffers[i]) vkDestroyBuffer(ctx->device, ctx->buffers[i], NULL);
            if (ctx->memories[i]) vkFreeMemory(ctx->device, ctx->memories[i], NULL);
        }
        vkDestroyDevice(ctx->device, NULL);
    }
    if (ctx->instance) vkDestroyInstance(ctx->instance, NULL);
    memset(ctx, 0, sizeof(*ctx));
}

static int create_context(ColiVulkanNativeContext *ctx) {
    memset(ctx, 0, sizeof(*ctx));

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "colibri-vulkan-native";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "colibri";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instance_info = {0};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    if (vkCreateInstance(&instance_info, NULL, &ctx->instance) != VK_SUCCESS) {
        return 0;
    }

    uint32_t device_count = 0;
    if (vkEnumeratePhysicalDevices(ctx->instance, &device_count, NULL) != VK_SUCCESS || device_count == 0) {
        destroy_context(ctx);
        return 0;
    }
    VkPhysicalDevice devices[8] = {0};
    if (vkEnumeratePhysicalDevices(ctx->instance, &device_count, devices) != VK_SUCCESS) {
        destroy_context(ctx);
        return 0;
    }

    for (uint32_t i = 0; i < device_count; ++i) {
        VkPhysicalDeviceProperties props = {0};
        vkGetPhysicalDeviceProperties(devices[i], &props);
        uint32_t family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &family_count, NULL);
        VkQueueFamilyProperties families[8] = {0};
        if (family_count > 8) family_count = 8;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &family_count, families);
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
    if (vkCreateDevice(ctx->physical_device, &device_info, NULL, &ctx->device) != VK_SUCCESS) {
        destroy_context(ctx);
        return 0;
    }
    vkGetDeviceQueue(ctx->device, ctx->queue_family_index, 0, &ctx->queue);

    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = ctx->queue_family_index;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(ctx->device, &pool_info, NULL, &ctx->command_pool) != VK_SUCCESS) {
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
    if (vkCreateDescriptorSetLayout(ctx->device, &layout_info, NULL, &ctx->descriptor_layout) != VK_SUCCESS) {
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
    if (vkCreateShaderModule(ctx->device, &shader_info, NULL, &ctx->shader_module) != VK_SUCCESS) {
        free(shader_code);
        destroy_context(ctx);
        return 0;
    }
    free(shader_code);

    VkPushConstantRange push_range = {0};
    push_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_range.offset = 0;
    push_range.size = sizeof(ColiPushConstants);

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &ctx->descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_range;
    if (vkCreatePipelineLayout(ctx->device, &pipeline_layout_info, NULL, &ctx->pipeline_layout) != VK_SUCCESS) {
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
    if (vkCreateComputePipelines(ctx->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &ctx->pipeline) != VK_SUCCESS) {
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
    if (vkCreateDescriptorPool(ctx->device, &pool_info2, NULL, &ctx->descriptor_pool) != VK_SUCCESS) {
        destroy_context(ctx);
        return 0;
    }

    VkDescriptorSetAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = ctx->descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &ctx->descriptor_layout;
    if (vkAllocateDescriptorSets(ctx->device, &alloc_info, &ctx->descriptor_set) != VK_SUCCESS) {
        destroy_context(ctx);
        return 0;
    }

    VkCommandBufferAllocateInfo cmd_alloc = {0};
    cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc.commandPool = ctx->command_pool;
    cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_alloc.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(ctx->device, &cmd_alloc, &ctx->command_buffer) != VK_SUCCESS) {
        destroy_context(ctx);
        return 0;
    }

    ctx->initialized = true;
    return 1;
}

static int create_buffer(ColiVulkanNativeContext *ctx, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory) {
    VkBufferCreateInfo info = {0};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(ctx->device, &info, NULL, buffer) != VK_SUCCESS) return 0;

    VkMemoryRequirements requirements = {0};
    vkGetBufferMemoryRequirements(ctx->device, *buffer, &requirements);
    uint32_t index = find_memory_type(ctx->physical_device, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (index == UINT32_MAX) {
        vkDestroyBuffer(ctx->device, *buffer, NULL);
        return 0;
    }
    VkMemoryAllocateInfo alloc = {0};
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.allocationSize = requirements.size;
    alloc.memoryTypeIndex = index;
    if (vkAllocateMemory(ctx->device, &alloc, NULL, memory) != VK_SUCCESS) {
        vkDestroyBuffer(ctx->device, *buffer, NULL);
        return 0;
    }
    if (vkBindBufferMemory(ctx->device, *buffer, *memory, 0) != VK_SUCCESS) {
        vkFreeMemory(ctx->device, *memory, NULL);
        vkDestroyBuffer(ctx->device, *buffer, NULL);
        return 0;
    }
    return 1;
}

static int run_vulkan_matmul(ColiVulkanNativeContext *ctx, const void *weights, const float *scales, int fmt, int S, int I, int O, float *y, const float *x) {
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
    if (!weights_t) { free(weights_f32); return 0; }
    for (int o = 0; o < O; ++o) {
        for (int i = 0; i < I; ++i) {
            weights_t[(size_t)i * (size_t)O + o] = weights_f32[(size_t)o * (size_t)I + i];
        }
    }

    if (!create_buffer(ctx, a_bytes, &ctx->buffers[0], &ctx->memories[0])) { free(weights_t); free(weights_f32); return 0; }
    if (!create_buffer(ctx, b_bytes, &ctx->buffers[1], &ctx->memories[1])) { free(weights_t); free(weights_f32); return 0; }
    if (!create_buffer(ctx, c_bytes, &ctx->buffers[2], &ctx->memories[2])) { free(weights_t); free(weights_f32); return 0; }

    void *mapped_a = NULL;
    void *mapped_b = NULL;
    void *mapped_c = NULL;
    if (vkMapMemory(ctx->device, ctx->memories[0], 0, a_bytes, 0, &mapped_a) != VK_SUCCESS ||
        vkMapMemory(ctx->device, ctx->memories[1], 0, b_bytes, 0, &mapped_b) != VK_SUCCESS ||
        vkMapMemory(ctx->device, ctx->memories[2], 0, c_bytes, 0, &mapped_c) != VK_SUCCESS) {
        vkUnmapMemory(ctx->device, ctx->memories[0]);
        vkUnmapMemory(ctx->device, ctx->memories[1]);
        vkUnmapMemory(ctx->device, ctx->memories[2]);
        free(weights_t); free(weights_f32); return 0;
    }
    memcpy(mapped_a, x, a_bytes);
    memcpy(mapped_b, weights_t, b_bytes);
    memset(mapped_c, 0, c_bytes);
    vkUnmapMemory(ctx->device, ctx->memories[0]);
    vkUnmapMemory(ctx->device, ctx->memories[1]);
    vkUnmapMemory(ctx->device, ctx->memories[2]);

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
    vkUpdateDescriptorSets(ctx->device, 3, writes, 0, NULL);

    VkCommandBufferBeginInfo begin = {0};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(ctx->command_buffer, &begin) != VK_SUCCESS) {
        free(weights_t); free(weights_f32); return 0;
    }
    vkCmdBindPipeline(ctx->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, ctx->pipeline);
    vkCmdBindDescriptorSets(ctx->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, ctx->pipeline_layout, 0, 1, &ctx->descriptor_set, 0, NULL);
    ColiPushConstants push = {S, O, I};
    vkCmdPushConstants(ctx->command_buffer, ctx->pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
    const uint32_t group_x = (O + 15) / 16;
    const uint32_t group_y = (S + 15) / 16;
    vkCmdDispatch(ctx->command_buffer, group_x, group_y, 1);
    if (vkEndCommandBuffer(ctx->command_buffer) != VK_SUCCESS) {
        free(weights_t); free(weights_f32); return 0;
    }

    VkSubmitInfo submit = {0};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &ctx->command_buffer;
    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (vkCreateFence(ctx->device, &fence_info, NULL, &fence) != VK_SUCCESS) {
        free(weights_t); free(weights_f32); return 0;
    }
    int ok = (vkQueueSubmit(ctx->queue, 1, &submit, fence) == VK_SUCCESS && vkWaitForFences(ctx->device, 1, &fence, VK_TRUE, UINT64_MAX) == VK_SUCCESS);
    vkDestroyFence(ctx->device, fence, NULL);
    if (!ok) { free(weights_t); free(weights_f32); return 0; }

    void *mapped_out = NULL;
    if (vkMapMemory(ctx->device, ctx->memories[2], 0, c_bytes, 0, &mapped_out) != VK_SUCCESS) {
        free(weights_t); free(weights_f32); return 0;
    }
    memcpy(y, mapped_out, c_bytes);
    vkUnmapMemory(ctx->device, ctx->memories[2]);

    if (scales && fmt != 0) {
        for (int s = 0; s < S; ++s) {
            for (int o = 0; o < O; ++o) {
                y[(size_t)s * (size_t)O + o] *= scales[o];
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        if (ctx->buffers[i]) vkDestroyBuffer(ctx->device, ctx->buffers[i], NULL);
        if (ctx->memories[i]) vkFreeMemory(ctx->device, ctx->memories[i], NULL);
        ctx->buffers[i] = VK_NULL_HANDLE;
        ctx->memories[i] = VK_NULL_HANDLE;
    }
    free(weights_t);
    free(weights_f32);
    return 1;
}

int coli_vulkan_native_init(void) {
    return 1;
}

void coli_vulkan_native_shutdown(void) {
}

int coli_vulkan_native_matmul(const void *weights, const float *scales, int fmt, int S, int I, int O, float *y, const float *x, int device) {
    (void)device;
    if (!weights || !y || !x || S <= 0 || I <= 0 || O <= 0 || (fmt != 0 && fmt != 1)) {
        return 0;
    }

    ColiVulkanNativeContext ctx;
    if (!create_context(&ctx)) {
        cpu_matmul(y, x, weights, scales, fmt, S, I, O);
        return 1;
    }
    if (!run_vulkan_matmul(&ctx, weights, scales, fmt, S, I, O, y, x)) {
        destroy_context(&ctx);
        cpu_matmul(y, x, weights, scales, fmt, S, I, O);
        return 1;
    }
    destroy_context(&ctx);
    return 1;
}
