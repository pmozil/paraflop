#pragma once
#include "common.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/descriptor_set.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/graphics_pipeline.hpp"
#include "vulkan_utils/index_buffer.hpp"
#include "vulkan_utils/swap_chain.hpp"
#include "vulkan_utils/uniform_buffer.hpp"
#include "vulkan_utils/vertex_buffer.hpp"

namespace renderer {
template <typename GraphicsPipeline> class Renderer {
    static_assert(
        std::is_base_of<graphics_pipeline::AbstractGraphicsPipeline,
                        GraphicsPipeline>::value,
        "The graphics pipeline type must inherit for AbstractGraphicsPipeline");

  public:
    Renderer(
        GLFWwindow *window, VkInstance m_instance, VkSurfaceKHR m_surface,
        std::shared_ptr<device::DeviceHandler> m_deviceHandler,
        std::shared_ptr<swap_chain::SwapChain> m_swapChain,
        std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer,
        std::shared_ptr<GraphicsPipeline> m_graphicsPipeline,
        std::shared_ptr<descriptor_set::DescriptorSetHandler>
            m_descriptorSetHandler,
        std::shared_ptr<buffer::VertexBuffer> m_vertexBuffer,
        std::shared_ptr<buffer::IndexBuffer> m_indexBuffer)
        : window(window), m_instance(m_instance), m_surface(m_surface),
          m_deviceHandler(std::move(m_deviceHandler)),
          m_commandBuffer(std::move(m_commandBuffer)),
          m_swapChain(std::move(m_swapChain)),
          m_graphicsPipeline(std::move(m_graphicsPipeline)),
          m_descriptorSetHandler(std::move(m_descriptorSetHandler)),
          m_vertexBuffer(std::move(m_vertexBuffer)),
          m_indexBuffer(std::move(m_indexBuffer)) {
        m_createSyncObjects();
    }
    ~Renderer() { cleanup(); }
    void handleWindowUpdate();
    void drawFrame();
    void cleanup();
    inline void setUniformBuffer(buffer::UniformBuffer buf) {
        m_uniformBuffer = buf;
    }

  private:
    GLFWwindow *window;
    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer;
    std::shared_ptr<swap_chain::SwapChain> m_swapChain;
    std::shared_ptr<GraphicsPipeline> m_graphicsPipeline;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    std::shared_ptr<descriptor_set::DescriptorSetHandler>
        m_descriptorSetHandler;
    std::shared_ptr<buffer::VertexBuffer> m_vertexBuffer;
    std::shared_ptr<buffer::IndexBuffer> m_indexBuffer;
    buffer::UniformBuffer m_uniformBuffer;
    size_t m_currentFrame = 0;
    bool m_framebufferResized = false;
    void m_createSyncObjects();
    void m_recordCommandBuffers(uint32_t imageIndex);
};
} // namespace renderer

#include "renderer/renderer_impl.hpp"
