#include "renderer.hpp"

renderer::Renderer::Renderer(
    GLFWwindow *window, VkInstance &instance, VkSurfaceKHR &surface,
    device::DeviceHandler *deviceHandler, swap_chain::SwapChain *swapChain,
    command_buffer::CommandBufferHandler *commandBuffer,
    graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline)
    : window(window), instance(instance), surface(surface),
      deviceHandler(deviceHandler), commandBuffer(commandBuffer),
      swapChain(swapChain), graphicsPipeline(graphicsPipeline) {
    createSyncObjects();
}

void renderer::Renderer::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChain->getSwapChainImages().size(),
                          VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(deviceHandler->getLogicalDevice(), &semaphoreInfo,
                              nullptr,
                              &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(deviceHandler->getLogicalDevice(), &semaphoreInfo,
                              nullptr,
                              &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(deviceHandler->getLogicalDevice(), &fenceInfo,
                          nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create synchronization objects for a frame!");
        }
    }
}

void renderer::Renderer::handleWindowUpdate() {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(deviceHandler->getLogicalDevice());

    swapChain->cleanup();
    commandBuffer->cleanup();
    graphicsPipeline->cleanup();

    swapChain->createSwapChain();
    swapChain->createImageViews();
    swapChain->createRenderPass();
    swapChain->createFrameBuffers();
    graphicsPipeline->createGraphicsPipeline();
    commandBuffer->createCommandBuffers();
}
