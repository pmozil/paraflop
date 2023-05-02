#include "renderer.hpp"

renderer::Renderer::Renderer(GLFWwindow *window, VkInstance *instance,
                             VkSurfaceKHR *surface,
                             graphics_pipeline::pipeline_type pipelineType)
    : window(window), instance(instance), surface(surface),
      deviceHandler(DEVICE_EXTENSIONS, VALIDATION_LAYERS, instance, surface),
      commandBuffer(&deviceHandler),
      swapChain(window, surface, &deviceHandler) {
    switch (pipelineType) {
    case graphics_pipeline::pipeline_type::PARAFLOP_GRAPHICS_PIPELINE_RASTER: {
        graphicsPipeline = new graphics_pipeline::RasterGraphicsPipeline(
            &swapChain, &deviceHandler);
    }
    }
}

renderer::Renderer::Renderer(
    GLFWwindow *window, VkInstance *instance, VkSurfaceKHR *surface,
    device::DeviceHandler deviceHandler, swap_chain::SwapChain swapChain,
    command_buffer::CommandBuffer commandBuffer,
    graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline)
    : window(window), instance(instance), surface(surface),
      deviceHandler(std::move(deviceHandler)), commandBuffer(commandBuffer),
      swapChain(std::move(swapChain)), graphicsPipeline(graphicsPipeline) {}

void renderer::Renderer::recordCommandBuffer(uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer.getCommandBuffer(), &beginInfo) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain.getRenderPass();
    renderPassInfo.framebuffer =
        swapChain.getSwapChainFrameBuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain.getSwapChainExtent();

    VkClearValue clearColor = {{{0.0F, 0.0F, 0.0F, 1.0F}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer.getCommandBuffer(), &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
}
