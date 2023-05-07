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
        deviceHandler->getQueueFamilyIndices(deviceHandler->physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    VK_CHECK(vkCreateCommandPool(deviceHandler->logicalDevice, &poolInfo,
                                 nullptr, &commandPool));
}

void command_buffer::CommandBufferHandler::createCommandBuffers() {
    commandBuffers.resize(swapChain->swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    VK_CHECK(vkAllocateCommandBuffers(deviceHandler->logicalDevice, &allocInfo,
                                      commandBuffers.data()))

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapChain->getRenderPass();
        renderPassInfo.framebuffer = swapChain->swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain->swapChainExtent;

        VkClearValue clearColor = {0.0F, 0.0F, 0.0F, 1.0F};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphicsPipeline->getGraphicsPipeline());

        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
    }
}

void command_buffer::CommandBufferHandler::cleanup() {
    vkFreeCommandBuffers(deviceHandler->logicalDevice, commandPool,
                         static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());
    commandBuffers.clear();
}
