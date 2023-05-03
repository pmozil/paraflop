#include "command_buffer.hpp"

command_buffer::CommandBuffer::CommandBuffer(
    device::DeviceHandler *deviceHandler)
    : deviceHandler(deviceHandler) {
    createCommandPool();
    createCommandBuffer();
}

void command_buffer::CommandBuffer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices =
        deviceHandler->getQueueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    if (vkCreateCommandPool(deviceHandler->getLogicalDevice(), &poolInfo,
                            nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void command_buffer::CommandBuffer::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(deviceHandler->getLogicalDevice(), &allocInfo,
                                 &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
