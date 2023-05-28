#pragma once
#include "renderer/renderer.hpp"
#include <iostream>

namespace renderer {
template <typename GraphicsPipeline>
void Renderer<GraphicsPipeline>::m_createSyncObjects() {
  m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  m_imagesInFlight.resize(m_swapChain->swapChainImages.size(), VK_NULL_HANDLE);

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

  m_swapChain->createSwapChain();
  m_swapChain->createRenderPass();
  m_swapChain->createImageViews();
  m_swapChain->createFrameBuffers();
}

template <typename GraphicsPipeline>
void Renderer<GraphicsPipeline>::drawFrame() {
  vkWaitForFences(m_deviceHandler->logicalDevice, 1,
                  &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      m_deviceHandler->logicalDevice, m_swapChain->swapChain, UINT64_MAX,
      m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    handleWindowUpdate();
    return;
  }
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  // updateUniformBuffer(m_currentFrame);

  vkResetFences(m_deviceHandler->logicalDevice, 1,
                &m_inFlightFences[m_currentFrame]);

  vkResetCommandBuffer(m_commandBuffer->commandBuffers[m_currentFrame],
                       /*VkCommandBufferResetFlagBits*/ 0);
  m_recordCommandBuffers(imageIndex);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  std::array<VkSemaphore, 1> waitSemaphores = {
      m_imageAvailableSemaphores[m_currentFrame]};
  std::array<VkPipelineStageFlags, 1> waitStages = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores.data();
  submitInfo.pWaitDstStageMask = waitStages.data();

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &m_commandBuffer->commandBuffers[m_currentFrame];

  std::array<VkSemaphore, 1> signalSemaphores = {
      m_renderFinishedSemaphores[m_currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores.data();

  VK_CHECK(vkQueueSubmit(m_deviceHandler->graphicsQueue, 1, &submitInfo,
                         m_inFlightFences[m_currentFrame]));

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores.data();

  std::array<VkSwapchainKHR, 1> swapChains = {m_swapChain->swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains.data();

  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(m_deviceHandler->presentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    handleWindowUpdate();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

template <typename GraphicsPipeline>
void Renderer<GraphicsPipeline>::m_recordCommandBuffers(uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  VK_CHECK(vkBeginCommandBuffer(m_commandBuffer->commandBuffers[m_currentFrame],
                                &beginInfo));

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_swapChain->getRenderPass();
  renderPassInfo.framebuffer = m_swapChain->swapChainFramebuffers[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_swapChain->swapChainExtent;

  VkClearValue clearColor = {{{0.0F, 0.0F, 0.0F, 1.0F}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(m_commandBuffer->commandBuffers[m_currentFrame],
                       &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(m_commandBuffer->commandBuffers[m_currentFrame],
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_graphicsPipeline->graphicsPipeline);

  VkViewport viewport{};
  viewport.x = 0.0F;
  viewport.y = 0.0F;
  viewport.width = (float)m_swapChain->swapChainExtent.width;
  viewport.height = (float)m_swapChain->swapChainExtent.height;
  viewport.minDepth = 0.0F;
  viewport.maxDepth = 1.0F;
  vkCmdSetViewport(m_commandBuffer->commandBuffers[m_currentFrame], 0, 1,
                   &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = m_swapChain->swapChainExtent;
  vkCmdSetScissor(m_commandBuffer->commandBuffers[m_currentFrame], 0, 1,
                  &scissor);

  std::array<VkBuffer, 1> vertexBuffers = {m_vertexBuffer->buffer};
  std::array<VkDeviceSize, 1> offsets = {0};
  vkCmdBindVertexBuffers(m_commandBuffer->commandBuffers[m_currentFrame], 0, 1,
                         vertexBuffers.data(), offsets.data());

  vkCmdBindIndexBuffer(m_commandBuffer->commandBuffers[m_currentFrame],
                       m_indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT16);

  // std::cout << "DEBUG: " << sizeof(m_descriptorSetHandler->descriptorSet)
  //           << "\n";

  vkCmdBindDescriptorSets(m_commandBuffer->commandBuffers[m_currentFrame],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_graphicsPipeline->pipelineLayout, 0, 1,
                          &m_descriptorSetHandler->descriptorSet, 0, nullptr);

  vkCmdDrawIndexed(
      m_commandBuffer->commandBuffers[m_currentFrame],
      static_cast<uint32_t>(m_indexBuffer->size / sizeof(uint16_t)), 1, 0, 0,
      0);

  vkCmdEndRenderPass(m_commandBuffer->commandBuffers[m_currentFrame]);

  VK_CHECK(vkEndCommandBuffer(m_commandBuffer->commandBuffers[m_currentFrame]));
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
