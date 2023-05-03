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
    ~CommandBufferHandler();
    inline VkCommandBuffer &getCommandBuffer(uint32_t n) {
        return commandBuffers[n];
    }
    inline std::vector<VkCommandBuffer> &getCommandBuffers() {
        return commandBuffers;
    }

  private:
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    device::DeviceHandler *deviceHandler;
    swap_chain::SwapChain *swapChain;
    graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline;
    void createCommandPool();
    void createCommandBuffers();
};
} // namespace command_buffer
