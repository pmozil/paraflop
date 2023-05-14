#pragma once
#include "vulkan_utils/common.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/graphics_pipeline.hpp"
#include "vulkan_utils/swap_chain.hpp"

namespace command_buffer {
class CommandBufferHandler {
  public:
    CommandBufferHandler(
        device::DeviceHandler *m_devicehandler,
        swap_chain::SwapChain *m_swapChain,
        graphics_pipeline::AbstractGraphicsPipeline *m_graphicsPipeline);
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    VkCommandBuffer transferBuffer = VK_NULL_HANDLE;
    void createCommandBuffers();
    void cleanupCommandBuffers();
    void cleanup();

  private:
    device::DeviceHandler *m_devicehandler;
    swap_chain::SwapChain *m_swapChain;
    graphics_pipeline::AbstractGraphicsPipeline *m_graphicsPipeline;
    void m_createCommandPool();
};
} // namespace command_buffer
