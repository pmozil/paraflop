#pragma once
#include "command_buffer.hpp"
#include "common.hpp"
#include "device.hpp"
#include "graphics_pipeline.hpp"
#include "swap_chain.hpp"

namespace renderer {
class Renderer {
  public:
    Renderer(GLFWwindow *window, VkInstance &instance, VkSurfaceKHR &surface,
             device::DeviceHandler *deviceHandler,
             swap_chain::SwapChain *swapChain,
             command_buffer::CommandBufferHandler *commandBuffer,
             graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline);
    void recordCommandBuffer(uint32_t imageIndex);
    void recordCommandBuffer(uint32_t imageIndex, uint32_t renderPassIndex);

  private:
    GLFWwindow *window;
    VkInstance &instance;
    VkSurfaceKHR &surface;
    device::DeviceHandler *deviceHandler;
    command_buffer::CommandBufferHandler *commandBuffer;
    swap_chain::SwapChain *swapChain;
    graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
    bool framebufferResized = false;
};
} // namespace renderer
