#pragma once
#include "common.hpp"

namespace create_info {
VkBufferCreateInfo
bufferCreateInfo(VkDeviceSize size, VkBufferUsageFlags usageFlags,
                 VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

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

VkImageViewCreateInfo imageViewCreateInfo(VkImage image, VkFormat format);

VkSwapchainCreateInfoKHR
swapChainCreateInfo(VkSurfaceKHR surface, uint32_t imageCount, VkFormat format,
                    VkColorSpaceKHR colorSpace, VkExtent2D extent,
                    uint32_t imageArrayLayers, VkImageUsageFlags usageFlags,
                    QueueFamilyIndices indices,
                    VkSurfaceTransformFlagBitsKHR currentTransform,
                    VkPresentModeKHR presentMode);

VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(VkDescriptorType type,
                                                    uint32_t poolSizeInt);

VkDescriptorSetAllocateInfo
descriptorSetAllocInfo(VkDescriptorPool descriptorPool,
                       std::vector<VkDescriptorSetLayout> &layouts);

VkDescriptorSetLayoutBinding
descriptorSetLayoutBinding(VkDescriptorType descType,
                           VkShaderStageFlags stageFlags);

VkDescriptorSetLayoutCreateInfo
descriptorSetLayoutInfo(VkDescriptorType descType,
                        VkShaderStageFlags stageFlags);
} // namespace create_info
