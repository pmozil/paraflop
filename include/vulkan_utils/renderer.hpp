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
    Renderer(GLFWwindow *window, VkInstance m_instance, VkSurfaceKHR m_surface,
             device::DeviceHandler *m_deviceHandler,
             swap_chain::SwapChain *m_swapChain,
             command_buffer::CommandBufferHandler *m_commandBuffer,
             GraphicsPipeline *m_graphicsPipeline)
        : window(window), m_instance(m_instance), m_surface(m_surface),
          m_deviceHandler(m_deviceHandler), m_commandBuffer(m_commandBuffer),
          m_swapChain(m_swapChain), m_graphicsPipeline(m_graphicsPipeline) {
        m_createSyncObjects();
    }
    void handleWindowUpdate();
    void drawFrame();
    void cleanup();

  private:
    GLFWwindow *window;
    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    device::DeviceHandler *m_deviceHandler;
    command_buffer::CommandBufferHandler *m_commandBuffer;
    swap_chain::SwapChain *m_swapChain;
    GraphicsPipeline *m_graphicsPipeline;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    size_t m_currentFrame = 0;
    bool m_framebufferResized = false;
    void m_createSyncObjects();
};
} // namespace renderer

#include "renderer_impl.hpp"
