#include "command_buffer.hpp"

command_buffer::CommandBufferHandler::CommandBufferHandler(
    device::DeviceHandler *deviceHandler, swap_chain::SwapChain *swapChain,
    graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline)
    : deviceHandler(deviceHandler), swapChain(swapChain),
      graphicsPipeline(graphicsPipeline) {
    createCommandPool();
    createCommandBuffers();
}

void command_buffer::CommandBufferHandler::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices =
        deviceHandler->getQueueFamilyIndices(
            deviceHandler->getPhysicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    if (vkCreateCommandPool(deviceHandler->getLogicalDevice(), &poolInfo,
                            nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void command_buffer::CommandBufferHandler::createCommandBuffers() {
    commandBuffers.resize(swapChain->getSwapChainFrameBuffers().size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(deviceHandler->getLogicalDevice(), &allocInfo,
                                 commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapChain->getRenderPass();
        renderPassInfo.framebuffer = swapChain->getSwapChainFrameBuffers()[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

        VkClearValue clearColor = {0.0F, 0.0F, 0.0F, 1.0F};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphicsPipeline->getGraphicsPipeline());

        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void command_buffer::CommandBufferHandler::cleanup() {
    vkFreeCommandBuffers(deviceHandler->getLogicalDevice(), commandPool,
                         static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());
}
