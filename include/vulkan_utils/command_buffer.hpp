#pragma once
#include "common.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/graphics_pipeline.hpp"
#include "vulkan_utils/swap_chain.hpp"

namespace command_buffer {
class CommandBufferHandler {
  public:
    CommandBufferHandler(
        std::shared_ptr<device::DeviceHandler> m_devicehandler,
        std::shared_ptr<swap_chain::SwapChain> m_swapChain,
        std::shared_ptr<graphics_pipeline::AbstractGraphicsPipeline>
            m_graphicsPipeline);
    ~CommandBufferHandler() { cleanup(); }
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    VkCommandBuffer transferBuffer = VK_NULL_HANDLE;
    void createCommandBuffers();
    void cleanupCommandBuffers();
    void cleanup();

  private:
    std::shared_ptr<device::DeviceHandler> m_devicehandler;
    std::shared_ptr<swap_chain::SwapChain> m_swapChain;
    std::shared_ptr<graphics_pipeline::AbstractGraphicsPipeline>
        m_graphicsPipeline;
    void m_createCommandPool();
};
} // namespace command_buffer
