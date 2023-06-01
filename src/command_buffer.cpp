#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/create_info.hpp"

namespace command_buffer {
CommandBufferHandler::CommandBufferHandler(
    std::shared_ptr<device::DeviceHandler> m_devicehandler,
    std::shared_ptr<swap_chain::SwapChain> m_swapChain,
    std::shared_ptr<graphics_pipeline::AbstractGraphicsPipeline>
        m_graphicsPipeline)
    : m_devicehandler(std::move(m_devicehandler)),
      m_swapChain(std::move(m_swapChain)),
      m_graphicsPipeline(std::move(m_graphicsPipeline)) {
    m_createCommandPool();
    createCommandBuffers();
}

void CommandBufferHandler::m_createCommandPool() {
    QueueFamilyIndices queueFamilyIndices =
        m_devicehandler->getQueueFamilyIndices(m_devicehandler->physicalDevice);

    VkCommandPoolCreateInfo poolInfo = create_info::commandPoolCreateInfo(
        queueFamilyIndices.graphicsFamily.value());
    VK_CHECK(vkCreateCommandPool(m_devicehandler->logicalDevice, &poolInfo,
                                 nullptr, &commandPool));
}

void CommandBufferHandler::createCommandBuffers() {
    commandBuffers.resize(m_swapChain->swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo =
        create_info::commandBufferAllocInfo(commandPool, commandBuffers.size());
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(m_devicehandler->logicalDevice,
                                      &allocInfo, commandBuffers.data()))

    VkCommandBufferAllocateInfo transferAllocInfo =
        create_info::commandBufferAllocInfo(commandPool, 1);

    vkAllocateCommandBuffers(m_devicehandler->logicalDevice, &transferAllocInfo,
                             &transferBuffer);
}

void CommandBufferHandler::cleanupCommandBuffers() {
    vkFreeCommandBuffers(m_devicehandler->logicalDevice, commandPool,
                         static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());
    commandBuffers.clear();
}
void CommandBufferHandler::cleanup() {
    cleanupCommandBuffers();
    vkDestroyCommandPool(m_devicehandler->logicalDevice, commandPool, nullptr);
}

} // namespace command_buffer
