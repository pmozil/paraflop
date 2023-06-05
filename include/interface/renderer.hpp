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
/**
 * \class Renderer
 *
 * \tparam GraphicsPipeline The type of graphics pipeline to be used.
 *
 * \brief The main renderer class responsible for rendering frames.
 *
 * The Renderer class handles rendering operations using a specified graphics
 * pipeline. It manages synchronization objects, command buffers, and performs
 * rendering tasks.
 *
 * \note The GraphicsPipeline type must inherit from AbstractGraphicsPipeline.
 */
template <typename GraphicsPipeline> class Renderer {
    static_assert(std::is_base_of<graphics_pipeline::AbstractGraphicsPipeline,
                                  GraphicsPipeline>::value,
                  "The graphics pipeline type must inherit from "
                  "AbstractGraphicsPipeline");

  public:
    /**
     * \fn Renderer(GLFWwindow *window, VkInstance m_instance, VkSurfaceKHR
     * m_surface, std::shared_ptr<device::DeviceHandler> m_deviceHandler,
     * std::shared_ptr<swap_chain::SwapChain> m_swapChain,
     * std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer,
     * std::shared_ptr<GraphicsPipeline> m_graphicsPipeline,
     * std::shared_ptr<descriptor_set::DescriptorSetHandler>
     * m_descriptorSetHandler, std::shared_ptr<buffer::VertexBuffer>
     * m_vertexBuffer, std::shared_ptr<buffer::IndexBuffer> m_indexBuffer)
     *
     * \brief Constructs a Renderer object.
     *
     * \param window The GLFW window.
     * \param m_instance The Vulkan instance.
     * \param m_surface The Vulkan surface.
     * \param m_deviceHandler The device handler.
     * \param m_swapChain The swap chain.
     * \param m_commandBuffer The command buffer handler.
     * \param m_graphicsPipeline The graphics pipeline to be used.
     * \param m_descriptorSetHandler The descriptor set handler.
     * \param m_vertexBuffer The vertex buffer.
     * \param m_indexBuffer The index buffer.
     */
    Renderer(GLFWwindow *window, VkInstance m_instance, VkSurfaceKHR m_surface,
             std::shared_ptr<device::DeviceHandler> m_deviceHandler,
             std::shared_ptr<swap_chain::SwapChain> m_swapChain,
             std::shared_ptr<command_buffer::ImageCommandBufferHandler>
                 m_commandBuffer,
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
          m_indexBuffer(std::move(m_indexBuffer)) {}

    /**
     * \fn void handleWindowUpdate()
     *
     * \brief Handles window updates.
     *
     * This function should be called whenever the window is resized or receives
     * an update. It updates the frame size and sets a flag indicating that the
     * framebuffer needs to be resized.
     */
    void handleWindowUpdate();

    /**
     * \fn void drawFrame()
     *
     * \brief Draws a frame.
     *
     * This function performs the main rendering operations for a single frame.
     * It acquires an image from the swap chain, executes the command buffer,
     * and presents the image.
     */
    void drawFrame();

  private:
    GLFWwindow *window;
    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    std::shared_ptr<command_buffer::ImageCommandBufferHandler> m_commandBuffer;
    std::shared_ptr<swap_chain::SwapChain> m_swapChain;
    std::shared_ptr<GraphicsPipeline> m_graphicsPipeline;
    std::shared_ptr<descriptor_set::DescriptorSetHandler>
        m_descriptorSetHandler;
    std::shared_ptr<buffer::VertexBuffer> m_vertexBuffer;
    std::shared_ptr<buffer::IndexBuffer> m_indexBuffer;
    size_t m_currentFrame = 0;
    bool m_framebufferResized = false;

    /**
     * \fn void m_recordCommandBuffers(uint32_t imageIndex)
     *
     * \brief Records the command buffers.
     *
     * \param imageIndex The index of the swap chain image.
     *
     * This function records the command buffers for rendering.
     * It binds the pipeline, sets up the render pass, and records drawing
     * commands.
     */
    void m_recordCommandBuffers(uint32_t imageIndex);
};

} // namespace renderer

#include "renderer_impl.hpp"
