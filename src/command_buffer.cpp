#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/create_info.hpp"

namespace command_buffer {
CommandBufferHandler::CommandBufferHandler(
    device::DeviceHandler *deviceHandler, swap_chain::SwapChain *swapChain,
    graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline)
    : deviceHandler(deviceHandler), swapChain(swapChain),
      graphicsPipeline(graphicsPipeline) {
    createCommandPool();
    createCommandBuffers();
}

void CommandBufferHandler::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices =
        deviceHandler->getQueueFamilyIndices(deviceHandler->physicalDevice);

    VkCommandPoolCreateInfo poolInfo = create_info::commandPoolCreateInfo(
        queueFamilyIndices.graphicsFamily.value());
    VK_CHECK(vkCreateCommandPool(deviceHandler->logicalDevice, &poolInfo,
                                 nullptr, &commandPool));
}

void CommandBufferHandler::createCommandBuffers() {
    commandBuffers.resize(swapChain->swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo =
        create_info::commandBuffferAllocInfo(commandPool,
                                             commandBuffers.size());

    VK_CHECK(vkAllocateCommandBuffers(deviceHandler->logicalDevice, &allocInfo,
                                      commandBuffers.data()))

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo =
            create_info::commabdBufferBeginInfo();

        VK_CHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

        VkClearValue clearColor = {0.0F, 0.0F, 0.0F, 1.0F};

        VkRenderPassBeginInfo renderPassInfo = create_info::renderPassBeginInfo(
            swapChain->getRenderPass(), swapChain->swapChainFramebuffers[i],
            swapChain->swapChainExtent, &clearColor);

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphicsPipeline->getGraphicsPipeline());

        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
    }
}

void CommandBufferHandler::cleanupCommandBuffers() {
    vkFreeCommandBuffers(deviceHandler->logicalDevice, commandPool,
                         static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());
    commandBuffers.clear();
}
void CommandBufferHandler::cleanup() {
    cleanupCommandBuffers();
    vkDestroyCommandPool(deviceHandler->logicalDevice, commandPool, nullptr);
}

} // namespace command_buffer
