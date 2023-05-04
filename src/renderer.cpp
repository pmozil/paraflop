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
    swapChain->createRenderPass();
    swapChain->createImageViews();
    swapChain->createFrameBuffers();
    graphicsPipeline->createGraphicsPipeline();
    commandBuffer->createCommandBuffers();
}

void renderer::Renderer::drawFrame() {
    vkWaitForFences(deviceHandler->getLogicalDevice(), 1,
                    &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        deviceHandler->getLogicalDevice(), swapChain->getSwapChain(),
        UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        handleWindowUpdate();
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(deviceHandler->getLogicalDevice(), 1,
                        &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers =
        &commandBuffer->getCommandBuffers()[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(deviceHandler->getLogicalDevice(), 1,
                  &inFlightFences[currentFrame]);

    if (vkQueueSubmit(deviceHandler->getGraphicsQueue(), 1, &submitInfo,
                      inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(deviceHandler->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        framebufferResized) {
        framebufferResized = false;
        handleWindowUpdate();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
