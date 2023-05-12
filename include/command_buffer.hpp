#pragma once
#include "common.hpp"
#include "device.hpp"
#include "graphics_pipeline.hpp"
#include "swap_chain.hpp"

namespace command_buffer {
class CommandBufferHandler {
  public:
    CommandBufferHandler(
        device::DeviceHandler *deviceHandler, swap_chain::SwapChain *swapChain,
        graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline);
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    VkCommandBuffer transferBuffer = VK_NULL_HANDLE;
    void createCommandBuffers();
    void cleanupCommandBuffers();
    void cleanup();

  private:
    device::DeviceHandler *deviceHandler;
    swap_chain::SwapChain *swapChain;
    graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline;
    void createCommandPool();
};
} // namespace command_buffer
