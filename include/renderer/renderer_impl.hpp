#pragma once
#include "renderer/renderer.hpp"

namespace renderer {
template <typename GraphicsPipeline>
void Renderer<GraphicsPipeline>::m_createSyncObjects() {
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    m_imagesInFlight.resize(m_swapChain->swapChainImages.size(),
                            VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_deviceHandler->logicalDevice, &semaphoreInfo,
                              nullptr,
                              &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_deviceHandler->logicalDevice, &semaphoreInfo,
                              nullptr,
                              &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_deviceHandler->logicalDevice, &fenceInfo, nullptr,
                          &m_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create synchronization objects for a frame!");
        }
    }
}

template <typename GraphicsPipeline>
void Renderer<GraphicsPipeline>::handleWindowUpdate() {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_deviceHandler->logicalDevice);

    m_swapChain->cleanup();
    m_commandBuffer->cleanupCommandBuffers();
    m_graphicsPipeline->cleanup();

    m_swapChain->createSwapChain();
    m_swapChain->createRenderPass();
    m_swapChain->createImageViews();
    m_swapChain->createFrameBuffers();
    m_graphicsPipeline->createGraphicsPipeline();
    m_commandBuffer->createCommandBuffers();
}

template <typename GraphicsPipeline>
void Renderer<GraphicsPipeline>::drawFrame() {
    vkWaitForFences(m_deviceHandler->logicalDevice, 1,
                    &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_deviceHandler->logicalDevice, m_swapChain->swapChain, UINT64_MAX,
        m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        handleWindowUpdate();
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(m_deviceHandler->logicalDevice, 1,
                        &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::array<VkSemaphore, 1> waitSemaphores = {
        m_imageAvailableSemaphores[m_currentFrame]};
    std::array<VkPipelineStageFlags, 1> waitStages = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer->commandBuffers[imageIndex];

    std::array<VkSemaphore, 1> signalSemaphores{
        m_renderFinishedSemaphores[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    vkResetFences(m_deviceHandler->logicalDevice, 1,
                  &m_inFlightFences[m_currentFrame]);

    if (vkQueueSubmit(m_deviceHandler->graphicsQueue, 1, &submitInfo,
                      m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores.data();

    std::array<VkSwapchainKHR, 1> swapChains = {m_swapChain->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains.data();

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_deviceHandler->presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_framebufferResized) {
        m_framebufferResized = false;
        handleWindowUpdate();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

template <typename GraphicsPipeline>
void Renderer<GraphicsPipeline>::cleanup() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_deviceHandler->logicalDevice,
                           m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_deviceHandler->logicalDevice,
                           m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_deviceHandler->logicalDevice, m_inFlightFences[i],
                       nullptr);
    }
}
} // namespace renderer
