#pragma once
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/common.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/graphics_pipeline.hpp"
#include "vulkan_utils/swap_chain.hpp"

namespace renderer {
template <typename GraphicsPipeline> class Renderer {
    static_assert(
        std::is_base_of<graphics_pipeline::AbstractGraphicsPipeline,
                        GraphicsPipeline>::value,
        "The graphics pipeline type must inherit for AbstractGraphicsPipeline");

  public:
    Renderer(GLFWwindow *window, VkInstance &instance, VkSurfaceKHR &surface,
             device::DeviceHandler *deviceHandler,
             swap_chain::SwapChain *swapChain,
             command_buffer::CommandBufferHandler *commandBuffer,
             GraphicsPipeline *graphicsPipeline)
        : window(window), instance(instance), surface(surface),
          deviceHandler(deviceHandler), commandBuffer(commandBuffer),
          swapChain(swapChain), graphicsPipeline(graphicsPipeline) {
        createSyncObjects();
    }
    void handleWindowUpdate();
    void drawFrame();
    void cleanup();

  private:
    GLFWwindow *window;
    VkInstance &instance;
    VkSurfaceKHR &surface;
    device::DeviceHandler *deviceHandler;
    command_buffer::CommandBufferHandler *commandBuffer;
    swap_chain::SwapChain *swapChain;
    GraphicsPipeline *graphicsPipeline;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
    bool framebufferResized = false;
    void createSyncObjects();
};
} // namespace renderer

#include "renderer_impl.hpp"
