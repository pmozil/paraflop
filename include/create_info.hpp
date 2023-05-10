#pragma once
#include "common.hpp"

namespace create_info {
VkBufferCreateInfo bufferCreateInfo(VkDeviceSize size,
                                    VkBufferUsageFlags usageFlags);

VkMemoryAllocateInfo memoryAllocInfo(VkDeviceSize size, uint32_t memoryIndex);

VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex);

VkCommandBufferAllocateInfo
commandBuffferAllocInfo(VkCommandPool commandPool, uint32_t commandBufferCount);

VkCommandBufferBeginInfo commabdBufferBeginInfo();

VkSubmitInfo submitInfo(uint32_t commandBufferCount, VkCommandBuffer *buffers);

VkBufferCopy copyRegion(VkDeviceSize size);

VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass,
                                          VkFramebuffer framebuffer,
                                          VkExtent2D extent,
                                          const VkClearValue *clearColor);

VkDeviceQueueCreateInfo queueCreateInfo(uint32_t queueFamily,
                                        const float *queuePriority);

VkPipelineShaderStageCreateInfo
shaderStageInfo(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStage);

VkDeviceCreateInfo
deviceCreateInfo(std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos,
                 std::vector<const char *> &deviceExtensions,
                 std::vector<const char *> &validationLayers,
                 const VkPhysicalDeviceFeatures *deviceFeatures);
} // namespace create_info
