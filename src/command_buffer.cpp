#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/create_info.hpp"

namespace command_buffer {
CommandBufferHandler::CommandBufferHandler(
    std::shared_ptr<device::DeviceHandler> m_deviceHandler,
    std::shared_ptr<swap_chain::SwapChain> m_swapChain,
    std::shared_ptr<graphics_pipeline::AbstractGraphicsPipeline>
        m_graphicsPipeline)
    : m_deviceHandler(std::move(m_deviceHandler)),
      m_swapChain(std::move(m_swapChain)),
      m_graphicsPipeline(std::move(m_graphicsPipeline)) {
    m_createCommandPool();
    createCommandBuffers();
}

void CommandBufferHandler::m_createCommandPool() {
    QueueFamilyIndices queueFamilyIndices =
        m_deviceHandler->getQueueFamilyIndices(m_deviceHandler->physicalDevice);

    VkCommandPoolCreateInfo poolInfo = create_info::commandPoolCreateInfo(
        queueFamilyIndices.graphicsFamily.value());
    VK_CHECK(vkCreateCommandPool(m_deviceHandler->logicalDevice, &poolInfo,
                                 nullptr, &commandPool));
}

void CommandBufferHandler::createCommandBuffers() {
    commandBuffers.resize(m_swapChain->swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo =
        create_info::commandBufferAllocInfo(commandPool, commandBuffers.size());
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(m_deviceHandler->logicalDevice,
                                      &allocInfo, commandBuffers.data()))

    VkCommandBufferAllocateInfo transferAllocInfo =
        create_info::commandBufferAllocInfo(commandPool, 1);

    vkAllocateCommandBuffers(m_deviceHandler->logicalDevice, &transferAllocInfo,
                             &transferBuffer);
}

void CommandBufferHandler::cleanupCommandBuffers() {
    vkFreeCommandBuffers(m_deviceHandler->logicalDevice, commandPool,
                         static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());
    commandBuffers.clear();
}
void CommandBufferHandler::cleanup() {
    cleanupCommandBuffers();
    vkDestroyCommandPool(m_deviceHandler->logicalDevice, commandPool, nullptr);
}

[[nodiscard]] VkCommandPool CommandBufferHandler::createCommandPool(
    uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags) const {
    VkCommandPoolCreateInfo commandPoolCreateInfo =
        create_info::commandPoolCreateInfo(queueFamilyIndex);
    commandPoolCreateInfo.flags = createFlags;
    VkCommandPool commandPool;
    VK_CHECK(vkCreateCommandPool(*m_deviceHandler, &commandPoolCreateInfo,
                                 nullptr, &commandPool));
    return commandPool;
}

[[nodiscard]] VkCommandBuffer
CommandBufferHandler::createCommandBuffer(VkCommandBufferLevel level,
                                          bool begin) const {
    VkCommandBufferAllocateInfo allocInfo =
        create_info::commandBufferAllocInfo(commandPool, 1);
    allocInfo.level = level;

    VkCommandBuffer cmdBuffer;
    VK_CHECK(
        vkAllocateCommandBuffers(*m_deviceHandler, &allocInfo, &cmdBuffer));
    // If requested, also start recording for the new command buffer
    if (begin) {
        VkCommandBufferBeginInfo bufferBeginInfo{};
        bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &bufferBeginInfo));
    }
    return cmdBuffer;
}
void CommandBufferHandler::flushCommandBuffer(VkCommandBuffer buf,
                                              VkQueue queue, bool free) {
    if (buf == VK_NULL_HANDLE) {
        return;
    }

    VK_CHECK(vkEndCommandBuffer(buf));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buf;
    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;

    VkFence fence;
    VK_CHECK(vkCreateFence(*m_deviceHandler, &fenceInfo, nullptr, &fence));
    // Submit to the queue
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence));
    // Wait for the fence to signal that command buffer has finished executing
    VK_CHECK(vkWaitForFences(*m_deviceHandler, 1, &fence, VK_TRUE,
                             DEFAULT_FENCE_TIMEOUT));
    vkDestroyFence(*m_deviceHandler, fence, nullptr);
    if (free) {
        vkFreeCommandBuffers(*m_deviceHandler, commandPool, 1, &buf);
    }
}

} // namespace command_buffer
