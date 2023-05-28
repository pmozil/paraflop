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
        create_info::commandBuffferAllocInfo(commandPool,
                                             commandBuffers.size());
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(m_devicehandler->logicalDevice,
                                      &allocInfo, commandBuffers.data()))

    // VkClearValue clearColor = {0.0F, 0.0F, 0.0F, 1.0F};


    // for (size_t i = 0; i < commandBuffers.size(); i++) {
    //     VkCommandBufferBeginInfo beginInfo =
    //         create_info::commabdBufferBeginInfo();

    //     VK_CHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

    //     VkRenderPassBeginInfo renderPassInfo = create_info::renderPassBeginInfo(
    //         m_swapChain->getRenderPass(), m_swapChain->swapChainFramebuffers[i],
    //         m_swapChain->swapChainExtent, &clearColor);

    //     vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
    //                          VK_SUBPASS_CONTENTS_INLINE);

    //     vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
    //                       m_graphicsPipeline->graphicsPipeline);

    //     vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

    //     vkCmdEndRenderPass(commandBuffers[i]);

    //     VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
    // }

    VkCommandBufferAllocateInfo transferAllocInfo =
        create_info::commandBuffferAllocInfo(commandPool, 1);

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
