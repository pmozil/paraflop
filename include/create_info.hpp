#pragma once
#include "common.hpp"

namespace create_info {
inline VkBufferCreateInfo bufferCreateInfo(VkDeviceSize size,
                                           VkBufferUsageFlags usageFlags) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return bufferInfo;
}

inline VkMemoryAllocateInfo memoryAllocInfo(VkDeviceSize size,
                                            uint32_t memoryIndex) {
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = memoryIndex;
    return allocInfo;
}

inline VkCommandPoolCreateInfo
commandPoolCreateInfo(uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    return poolInfo;
}

inline VkCommandBufferAllocateInfo
commandBuffferAllocInfo(VkCommandPool commandPool,
                        uint32_t commandBufferCount) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBufferCount;
    return allocInfo;
}

inline VkCommandBufferBeginInfo commabdBufferbeginInfo() {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    return beginInfo;
}

inline VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass,
                                                 VkFramebuffer framebuffer,
                                                 VkExtent2D extent,
                                                 VkClearValue clearColor) {

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    return renderPassInfo;
}

inline VkDeviceQueueCreateInfo queueCreateInfo(uint32_t queueFamily,
                                               const float *queuePriority) {

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriority;
    return queueCreateInfo;
}

inline VkPipelineShaderStageCreateInfo
shaderStageInfo(VkShaderModule shaderModule,
                VkShaderStageFlagBits shaderStage) {
    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = shaderStage;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = "main";
    return shaderStageInfo;
}
} // namespace create_info
