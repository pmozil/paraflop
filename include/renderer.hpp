#pragma once
#include "command_buffer.hpp"
#include "common.hpp"
#include "device.hpp"
#include "graphics_pipeline.hpp"
#include "swap_chain.hpp"

namespace renderer {
class Renderer {
  public:
    Renderer(GLFWwindow *window, VkInstance *instance, VkSurfaceKHR *surface,
             graphics_pipeline::pipeline_type pipelineType);
    Renderer(GLFWwindow *window, VkInstance *instance, VkSurfaceKHR *surface,
             device::DeviceHandler deviceHandler,
             swap_chain::SwapChain swapChain,
             command_buffer::CommandBuffer commandBuffer,
             graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline);
    void recordCommandBuffer(uint32_t imageIndex);

  private:
    GLFWwindow *window;
    VkInstance *instance;
    VkSurfaceKHR *surface;
    device::DeviceHandler deviceHandler;
    command_buffer::CommandBuffer commandBuffer;
    swap_chain::SwapChain swapChain;
    graphics_pipeline::AbstractGraphicsPipeline *graphicsPipeline;
};
} // namespace renderer
