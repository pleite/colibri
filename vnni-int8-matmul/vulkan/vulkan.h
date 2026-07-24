#ifndef VNNI_VULKAN_SHIM_H
#define VNNI_VULKAN_SHIM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t VkFlags;
typedef uint32_t VkResult;
typedef uint32_t VkStructureType;
typedef uint64_t VkDeviceSize;
typedef uint64_t VkSampleMask;
typedef uint32_t VkBool32;
typedef uint64_t VkHandle;

typedef struct VkInstance_T *VkInstance;
typedef struct VkDevice_T *VkDevice;
typedef struct VkPhysicalDevice_T *VkPhysicalDevice;
typedef struct VkQueue_T *VkQueue;
typedef struct VkCommandPool_T *VkCommandPool;
typedef struct VkPipeline_T *VkPipeline;
typedef struct VkPipelineLayout_T *VkPipelineLayout;
typedef struct VkShaderModule_T *VkShaderModule;
typedef struct VkDescriptorSetLayout_T *VkDescriptorSetLayout;
typedef struct VkDescriptorPool_T *VkDescriptorPool;
typedef struct VkDescriptorSet_T *VkDescriptorSet;
typedef struct VkBuffer_T *VkBuffer;
typedef struct VkDeviceMemory_T *VkDeviceMemory;
typedef struct VkCommandBuffer_T *VkCommandBuffer;
typedef struct VkFence_T *VkFence;
typedef struct VkPipelineCache_T *VkPipelineCache;

typedef struct VkExtent3D { uint32_t width; uint32_t height; uint32_t depth; } VkExtent3D;
typedef struct VkPhysicalDeviceMemoryProperties { uint32_t memoryTypeCount; struct { uint32_t propertyFlags; } memoryTypes[32]; } VkPhysicalDeviceMemoryProperties;
typedef struct VkMemoryRequirements { uint32_t memoryTypeBits; VkDeviceSize size; VkDeviceSize alignment; } VkMemoryRequirements;
typedef struct VkQueueFamilyProperties { VkFlags queueFlags; uint32_t queueCount; uint32_t timestampValidBits; VkExtent3D minImageTransferGranularity; } VkQueueFamilyProperties;
typedef struct VkPhysicalDeviceProperties { uint32_t apiVersion; } VkPhysicalDeviceProperties;

typedef struct VkApplicationInfo { VkStructureType sType; const void *pNext; const char *pApplicationName; uint32_t applicationVersion; const char *pEngineName; uint32_t engineVersion; uint32_t apiVersion; } VkApplicationInfo;
typedef struct VkInstanceCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; const VkApplicationInfo *pApplicationInfo; uint32_t enabledLayerCount; const char *const *ppEnabledLayerNames; uint32_t enabledExtensionCount; const char *const *ppEnabledExtensionNames; } VkInstanceCreateInfo;
typedef struct VkDeviceQueueCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; uint32_t queueFamilyIndex; uint32_t queueCount; const float *pQueuePriorities; } VkDeviceQueueCreateInfo;
typedef struct VkDeviceCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; uint32_t queueCreateInfoCount; const VkDeviceQueueCreateInfo *pQueueCreateInfos; uint32_t enabledLayerCount; const char *const *ppEnabledLayerNames; uint32_t enabledExtensionCount; const char *const *ppEnabledExtensionNames; const void *pEnabledFeatures; } VkDeviceCreateInfo;
typedef struct VkCommandPoolCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; uint32_t queueFamilyIndex; } VkCommandPoolCreateInfo;
typedef struct VkDescriptorSetLayoutBinding { uint32_t binding; uint32_t descriptorType; uint32_t descriptorCount; uint32_t stageFlags; const void *pImmutableSamplers; } VkDescriptorSetLayoutBinding;
typedef struct VkDescriptorSetLayoutCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; uint32_t bindingCount; const VkDescriptorSetLayoutBinding *pBindings; } VkDescriptorSetLayoutCreateInfo;
typedef struct VkShaderModuleCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; size_t codeSize; const uint32_t *pCode; } VkShaderModuleCreateInfo;
typedef struct VkPushConstantRange { uint32_t stageFlags; uint32_t offset; uint32_t size; } VkPushConstantRange;
typedef struct VkPipelineLayoutCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; uint32_t setLayoutCount; const VkDescriptorSetLayout *pSetLayouts; uint32_t pushConstantRangeCount; const VkPushConstantRange *pPushConstantRanges; } VkPipelineLayoutCreateInfo;
typedef struct VkPipelineShaderStageCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; uint32_t stage; VkShaderModule module; const char *pName; const void *pSpecializationInfo; } VkPipelineShaderStageCreateInfo;
typedef struct VkComputePipelineCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; VkPipelineShaderStageCreateInfo stage; VkPipelineLayout layout; VkPipeline basePipelineHandle; int32_t basePipelineIndex; } VkComputePipelineCreateInfo;
typedef struct VkDescriptorPoolSize { uint32_t type; uint32_t descriptorCount; } VkDescriptorPoolSize;
typedef struct VkDescriptorPoolCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; uint32_t maxSets; uint32_t poolSizeCount; const VkDescriptorPoolSize *pPoolSizes; } VkDescriptorPoolCreateInfo;
typedef struct VkDescriptorSetAllocateInfo { VkStructureType sType; const void *pNext; VkDescriptorPool descriptorPool; uint32_t descriptorSetCount; const VkDescriptorSetLayout *pSetLayouts; } VkDescriptorSetAllocateInfo;
typedef struct VkCommandBufferAllocateInfo { VkStructureType sType; const void *pNext; VkCommandPool commandPool; uint32_t level; uint32_t commandBufferCount; } VkCommandBufferAllocateInfo;
typedef struct VkBufferCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; VkDeviceSize size; uint32_t usage; uint32_t sharingMode; uint32_t queueFamilyIndexCount; const uint32_t *pQueueFamilyIndices; } VkBufferCreateInfo;
typedef struct VkMemoryAllocateInfo { VkStructureType sType; const void *pNext; VkDeviceSize allocationSize; uint32_t memoryTypeIndex; } VkMemoryAllocateInfo;
typedef struct VkDescriptorBufferInfo { VkBuffer buffer; VkDeviceSize offset; VkDeviceSize range; } VkDescriptorBufferInfo;
typedef struct VkWriteDescriptorSet { VkStructureType sType; const void *pNext; VkDescriptorSet dstSet; uint32_t dstBinding; uint32_t dstArrayElement; uint32_t descriptorCount; uint32_t descriptorType; const VkDescriptorBufferInfo *pBufferInfo; const void *pImageInfo; const void *pTexelBufferView; } VkWriteDescriptorSet;
typedef struct VkCommandBufferBeginInfo { VkStructureType sType; const void *pNext; VkFlags flags; const void *pInheritanceInfo; } VkCommandBufferBeginInfo;
typedef struct VkSubmitInfo { VkStructureType sType; const void *pNext; uint32_t waitSemaphoreCount; const void *pWaitSemaphores; const uint32_t *pWaitDstStageMask; uint32_t commandBufferCount; VkCommandBuffer *pCommandBuffers; uint32_t signalSemaphoreCount; const void *pSignalSemaphores; } VkSubmitInfo;
typedef struct VkFenceCreateInfo { VkStructureType sType; const void *pNext; VkFlags flags; } VkFenceCreateInfo;

typedef uint32_t VkDescriptorType;
typedef uint32_t VkDescriptorTypeFlags;
typedef uint32_t VkBufferUsageFlags;
typedef uint32_t VkMemoryPropertyFlags;
typedef uint32_t VkShaderStageFlags;
typedef uint32_t VkCommandBufferUsageFlags;

typedef void (*PFN_vkVoidFunction)(void);
typedef PFN_vkVoidFunction (*PFN_vkGetInstanceProcAddr)(VkInstance, const char *);

typedef VkResult (*PFN_vkCreateInstance)(const VkInstanceCreateInfo *, const void *, VkInstance *);
typedef void (*PFN_vkDestroyInstance)(VkInstance, const void *);
typedef VkResult (*PFN_vkEnumeratePhysicalDevices)(VkInstance, uint32_t *, VkPhysicalDevice *);
typedef void (*PFN_vkGetPhysicalDeviceProperties)(VkPhysicalDevice, VkPhysicalDeviceProperties *);
typedef void (*PFN_vkGetPhysicalDeviceQueueFamilyProperties)(VkPhysicalDevice, uint32_t *, VkQueueFamilyProperties *);
typedef void (*PFN_vkGetPhysicalDeviceMemoryProperties)(VkPhysicalDevice, VkPhysicalDeviceMemoryProperties *);
typedef VkResult (*PFN_vkCreateDevice)(VkPhysicalDevice, const VkDeviceCreateInfo *, const void *, VkDevice *);
typedef void (*PFN_vkDestroyDevice)(VkDevice, const void *);
typedef void (*PFN_vkGetDeviceQueue)(VkDevice, uint32_t, uint32_t, VkQueue *);
typedef VkResult (*PFN_vkCreateCommandPool)(VkDevice, const VkCommandPoolCreateInfo *, const void *, VkCommandPool *);
typedef void (*PFN_vkDestroyCommandPool)(VkDevice, VkCommandPool, const void *);
typedef VkResult (*PFN_vkAllocateCommandBuffers)(VkDevice, const VkCommandBufferAllocateInfo *, VkCommandBuffer *);
typedef void (*PFN_vkFreeCommandBuffers)(VkDevice, VkCommandPool, uint32_t, const VkCommandBuffer *);
typedef VkResult (*PFN_vkBeginCommandBuffer)(VkCommandBuffer, const VkCommandBufferBeginInfo *);
typedef VkResult (*PFN_vkEndCommandBuffer)(VkCommandBuffer);
typedef VkResult (*PFN_vkCreateDescriptorSetLayout)(VkDevice, const VkDescriptorSetLayoutCreateInfo *, const void *, VkDescriptorSetLayout *);
typedef void (*PFN_vkDestroyDescriptorSetLayout)(VkDevice, VkDescriptorSetLayout, const void *);
typedef VkResult (*PFN_vkCreateShaderModule)(VkDevice, const VkShaderModuleCreateInfo *, const void *, VkShaderModule *);
typedef void (*PFN_vkDestroyShaderModule)(VkDevice, VkShaderModule, const void *);
typedef VkResult (*PFN_vkCreatePipelineLayout)(VkDevice, const VkPipelineLayoutCreateInfo *, const void *, VkPipelineLayout *);
typedef void (*PFN_vkDestroyPipelineLayout)(VkDevice, VkPipelineLayout, const void *);
typedef VkResult (*PFN_vkCreateComputePipelines)(VkDevice, VkPipelineCache, uint32_t, const VkComputePipelineCreateInfo *, const void *, VkPipeline *);
typedef void (*PFN_vkDestroyPipeline)(VkDevice, VkPipeline, const void *);
typedef VkResult (*PFN_vkCreateDescriptorPool)(VkDevice, const VkDescriptorPoolCreateInfo *, const void *, VkDescriptorPool *);
typedef void (*PFN_vkDestroyDescriptorPool)(VkDevice, VkDescriptorPool, const void *);
typedef VkResult (*PFN_vkAllocateDescriptorSets)(VkDevice, const VkDescriptorSetAllocateInfo *, VkDescriptorSet *);
typedef void (*PFN_vkUpdateDescriptorSets)(VkDevice, uint32_t, const VkWriteDescriptorSet *, uint32_t, const void *);
typedef VkResult (*PFN_vkCreateBuffer)(VkDevice, const VkBufferCreateInfo *, const void *, VkBuffer *);
typedef void (*PFN_vkDestroyBuffer)(VkDevice, VkBuffer, const void *);
typedef void (*PFN_vkGetBufferMemoryRequirements)(VkDevice, VkBuffer, VkMemoryRequirements *);
typedef VkResult (*PFN_vkAllocateMemory)(VkDevice, const VkMemoryAllocateInfo *, const void *, VkDeviceMemory *);
typedef void (*PFN_vkFreeMemory)(VkDevice, VkDeviceMemory, const void *);
typedef VkResult (*PFN_vkBindBufferMemory)(VkDevice, VkBuffer, VkDeviceMemory, VkDeviceSize);
typedef VkResult (*PFN_vkMapMemory)(VkDevice, VkDeviceMemory, VkDeviceSize, VkDeviceSize, VkFlags, void **);
typedef void (*PFN_vkUnmapMemory)(VkDevice, VkDeviceMemory);
typedef void (*PFN_vkCmdBindPipeline)(VkCommandBuffer, uint32_t, VkPipeline);
typedef void (*PFN_vkCmdBindDescriptorSets)(VkCommandBuffer, uint32_t, VkPipelineLayout, uint32_t, uint32_t, const VkDescriptorSet *, uint32_t, const uint32_t *);
typedef void (*PFN_vkCmdPushConstants)(VkCommandBuffer, VkPipelineLayout, uint32_t, uint32_t, uint32_t, const void *);
typedef void (*PFN_vkCmdDispatch)(VkCommandBuffer, uint32_t, uint32_t, uint32_t);
typedef VkResult (*PFN_vkQueueSubmit)(VkQueue, uint32_t, const VkSubmitInfo *, VkFence);
typedef VkResult (*PFN_vkWaitForFences)(VkDevice, uint32_t, const VkFence *, VkBool32, uint64_t);
typedef VkResult (*PFN_vkCreateFence)(VkDevice, const VkFenceCreateInfo *, const void *, VkFence *);
typedef void (*PFN_vkDestroyFence)(VkDevice, VkFence, const void *);

typedef struct { PFN_vkCreateInstance vkCreateInstance; PFN_vkDestroyInstance vkDestroyInstance; PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices; PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties; PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties; PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties; PFN_vkCreateDevice vkCreateDevice; PFN_vkDestroyDevice vkDestroyDevice; PFN_vkGetDeviceQueue vkGetDeviceQueue; PFN_vkCreateCommandPool vkCreateCommandPool; PFN_vkDestroyCommandPool vkDestroyCommandPool; PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers; PFN_vkFreeCommandBuffers vkFreeCommandBuffers; PFN_vkBeginCommandBuffer vkBeginCommandBuffer; PFN_vkEndCommandBuffer vkEndCommandBuffer; PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout; PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout; PFN_vkCreateShaderModule vkCreateShaderModule; PFN_vkDestroyShaderModule vkDestroyShaderModule; PFN_vkCreatePipelineLayout vkCreatePipelineLayout; PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout; PFN_vkCreateComputePipelines vkCreateComputePipelines; PFN_vkDestroyPipeline vkDestroyPipeline; PFN_vkCreateDescriptorPool vkCreateDescriptorPool; PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool; PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets; PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets; PFN_vkCreateBuffer vkCreateBuffer; PFN_vkDestroyBuffer vkDestroyBuffer; PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements; PFN_vkAllocateMemory vkAllocateMemory; PFN_vkFreeMemory vkFreeMemory; PFN_vkBindBufferMemory vkBindBufferMemory; PFN_vkMapMemory vkMapMemory; PFN_vkUnmapMemory vkUnmapMemory; PFN_vkCmdBindPipeline vkCmdBindPipeline; PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets; PFN_vkCmdPushConstants vkCmdPushConstants; PFN_vkCmdDispatch vkCmdDispatch; PFN_vkQueueSubmit vkQueueSubmit; PFN_vkWaitForFences vkWaitForFences; PFN_vkCreateFence vkCreateFence; PFN_vkDestroyFence vkDestroyFence; } VnnVulkanDispatch;

#define VK_NULL_HANDLE ((void *)0)
#define VK_MAKE_VERSION(major, minor, patch) (((uint32_t)(major) << 22) | ((uint32_t)(minor) << 12) | (uint32_t)(patch))
#define VK_API_VERSION_1_0 VK_MAKE_VERSION(1, 0, 0)
#define VK_API_VERSION_1_2 VK_MAKE_VERSION(1, 2, 0)
#define VK_STRUCTURE_TYPE_APPLICATION_INFO 0
#define VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO 1
#define VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO 2
#define VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO 3
#define VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO 4
#define VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO 5
#define VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO 6
#define VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO 7
#define VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO 8
#define VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO 9
#define VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO 10
#define VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO 11
#define VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO 12
#define VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO 13
#define VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO 14
#define VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET 15
#define VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO 16
#define VK_STRUCTURE_TYPE_SUBMIT_INFO 17
#define VK_STRUCTURE_TYPE_FENCE_CREATE_INFO 18
#define VK_TRUE 1
#define VK_FALSE 0
#define VK_QUEUE_COMPUTE_BIT 0x00000002
#define VK_SHADER_STAGE_COMPUTE_BIT 0x00000020
#define VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 0x00000004
#define VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 0x00000002
#define VK_DESCRIPTOR_TYPE_STORAGE_BUFFER 0
#define VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 0x00000004
#define VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT 0x00000002
#define VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT 0x00000001
#define VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT 0x00000001
#define VK_FENCE_CREATE_SIGNALED_BIT 0x00000001
#define VK_PIPELINE_BIND_POINT_COMPUTE 0
#define VK_SHARING_MODE_EXCLUSIVE 0
#define VK_COMMAND_BUFFER_LEVEL_PRIMARY 0
#define VK_SUCCESS 0
#define VK_ERROR_INITIALIZATION_FAILED 0x00000001
#define VK_WHOLE_SIZE ((VkDeviceSize)-1)

#ifdef __cplusplus
}
#endif

#endif
