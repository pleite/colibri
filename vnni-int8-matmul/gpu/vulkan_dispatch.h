#ifndef VNNI_VULKAN_DISPATCH_H
#define VNNI_VULKAN_DISPATCH_H

/**
 * vulkan_dispatch.h — dispatch table built from the *official* Vulkan headers.
 *
 * This file deliberately contains no hand-written Vulkan type, struct or enum
 * definitions. Every `PFN_vk*` typedef, every struct layout and every enum value
 * comes from <vulkan/vulkan_core.h> (the Khronos-generated header shipped by the
 * `vulkan-headers` / `libvulkan-dev` package).
 *
 * Rationale — see docs/vulkan_debug_attempts.md. A previous revision of this
 * project shipped a hand-rolled `vulkan/vulkan.h` whose struct layouts did not
 * match the Vulkan ABI. `VkPhysicalDeviceProperties` was declared as 4 bytes
 * while the real structure is 824 bytes, so the very first
 * `vkGetPhysicalDeviceProperties()` call smashed 820 bytes of the caller's
 * stack. That, and not `dlsym`, was the cause of the long-standing segfault.
 *
 * Rule for this file: if you need a new entry point, add a `PFN_` member. Never
 * re-declare a Vulkan type here.
 */

#include <vulkan/vulkan_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Instance-level entry points.
 *
 * Per the Vulkan loader specification ("Vulkan Loader Interface", section
 * "Application Interface to the Loader"), the only symbol an application may
 * legitimately obtain with dlsym() from libvulkan.so.1 is
 * vkGetInstanceProcAddr. Everything else must be resolved through the
 * vkGetInstanceProcAddr -> vkGetDeviceProcAddr chain so that the loader can
 * install its trampolines for the correct ICD.
 */
typedef struct VnnVulkanInstanceDispatch {
    PFN_vkGetInstanceProcAddr                       vkGetInstanceProcAddr;

    /* Resolved with a NULL instance (global commands). */
    PFN_vkCreateInstance                            vkCreateInstance;
    PFN_vkEnumerateInstanceVersion                  vkEnumerateInstanceVersion;

    /* Resolved with a valid VkInstance. */
    PFN_vkDestroyInstance                           vkDestroyInstance;
    PFN_vkEnumeratePhysicalDevices                  vkEnumeratePhysicalDevices;
    PFN_vkGetPhysicalDeviceProperties               vkGetPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties    vkGetPhysicalDeviceQueueFamilyProperties;
    PFN_vkGetPhysicalDeviceMemoryProperties         vkGetPhysicalDeviceMemoryProperties;
    PFN_vkCreateDevice                              vkCreateDevice;
    PFN_vkGetDeviceProcAddr                         vkGetDeviceProcAddr;
} VnnVulkanInstanceDispatch;

/* Device-level entry points, resolved through vkGetDeviceProcAddr. */
typedef struct VnnVulkanDeviceDispatch {
    PFN_vkDestroyDevice                             vkDestroyDevice;
    PFN_vkDeviceWaitIdle                            vkDeviceWaitIdle;
    PFN_vkGetDeviceQueue                            vkGetDeviceQueue;
    PFN_vkCreateCommandPool                         vkCreateCommandPool;
    PFN_vkDestroyCommandPool                        vkDestroyCommandPool;
    PFN_vkAllocateCommandBuffers                    vkAllocateCommandBuffers;
    PFN_vkFreeCommandBuffers                        vkFreeCommandBuffers;
    PFN_vkBeginCommandBuffer                        vkBeginCommandBuffer;
    PFN_vkEndCommandBuffer                          vkEndCommandBuffer;
    PFN_vkResetCommandBuffer                        vkResetCommandBuffer;
    PFN_vkCreateDescriptorSetLayout                 vkCreateDescriptorSetLayout;
    PFN_vkDestroyDescriptorSetLayout                vkDestroyDescriptorSetLayout;
    PFN_vkCreateShaderModule                        vkCreateShaderModule;
    PFN_vkDestroyShaderModule                       vkDestroyShaderModule;
    PFN_vkCreatePipelineLayout                      vkCreatePipelineLayout;
    PFN_vkDestroyPipelineLayout                     vkDestroyPipelineLayout;
    PFN_vkCreateComputePipelines                    vkCreateComputePipelines;
    PFN_vkDestroyPipeline                           vkDestroyPipeline;
    PFN_vkCreateDescriptorPool                      vkCreateDescriptorPool;
    PFN_vkDestroyDescriptorPool                     vkDestroyDescriptorPool;
    PFN_vkAllocateDescriptorSets                    vkAllocateDescriptorSets;
    PFN_vkUpdateDescriptorSets                      vkUpdateDescriptorSets;
    PFN_vkCreateBuffer                              vkCreateBuffer;
    PFN_vkDestroyBuffer                             vkDestroyBuffer;
    PFN_vkGetBufferMemoryRequirements               vkGetBufferMemoryRequirements;
    PFN_vkAllocateMemory                            vkAllocateMemory;
    PFN_vkFreeMemory                                vkFreeMemory;
    PFN_vkBindBufferMemory                          vkBindBufferMemory;
    PFN_vkMapMemory                                 vkMapMemory;
    PFN_vkUnmapMemory                               vkUnmapMemory;
    PFN_vkCmdBindPipeline                           vkCmdBindPipeline;
    PFN_vkCmdBindDescriptorSets                     vkCmdBindDescriptorSets;
    PFN_vkCmdPushConstants                          vkCmdPushConstants;
    PFN_vkCmdPipelineBarrier                        vkCmdPipelineBarrier;
    PFN_vkCmdDispatch                               vkCmdDispatch;
    PFN_vkQueueSubmit                               vkQueueSubmit;
    PFN_vkCreateFence                               vkCreateFence;
    PFN_vkDestroyFence                              vkDestroyFence;
    PFN_vkWaitForFences                             vkWaitForFences;
    PFN_vkResetFences                               vkResetFences;
} VnnVulkanDeviceDispatch;

#ifdef __cplusplus
}
#endif

#endif /* VNNI_VULKAN_DISPATCH_H */
