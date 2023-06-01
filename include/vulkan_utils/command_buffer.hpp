#pragma once
#include "common.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/graphics_pipeline.hpp"
#include "vulkan_utils/swap_chain.hpp"

namespace command_buffer {
/**
 * \class CommandBufferHandler
 * \brief Wrapper class for Vulkan command buffer handling.
 *
 * The CommandBufferHandler class provides an interface for managing Vulkan
 * command buffers.
 */
class CommandBufferHandler {
  public:
    /**
     * \brief Constructs a CommandBufferHandler object.
     *
     * \param m_devicehandler The device handler used for command buffer
     * operations. \param m_swapChain The swap chain associated with the command
     * buffers. \param m_graphicsPipeline The graphics pipeline associated with
     * the command buffers.
     */
    CommandBufferHandler(
        std::shared_ptr<device::DeviceHandler> m_devicehandler,
        std::shared_ptr<swap_chain::SwapChain> m_swapChain,
        std::shared_ptr<graphics_pipeline::AbstractGraphicsPipeline>
            m_graphicsPipeline);

    /**
     * \brief Destructor for the CommandBufferHandler object.
     *
     * Cleans up the command buffer handler resources.
     */
    ~CommandBufferHandler() { cleanup(); }

    VkCommandPool commandPool; /**< The Vulkan command pool. */
    std::vector<VkCommandBuffer>
        commandBuffers; /**< The Vulkan command buffers. */
    VkCommandBuffer transferBuffer =
        VK_NULL_HANDLE; /**< The Vulkan command buffer for transfer operations.
                         */

    /**
     * \brief Creates the Vulkan command buffers.
     *
     * This function creates the Vulkan command buffers for rendering.
     */
    void createCommandBuffers();

    /**
     * \brief Cleans up the Vulkan command buffers.
     *
     * This function frees the Vulkan command buffers.
     */
    void cleanupCommandBuffers();

    /**
     * \brief Cleans up the command buffer handler resources.
     *
     * This function destroys the Vulkan command pool.
     */
    void cleanup();

  private:
    std::shared_ptr<device::DeviceHandler>
        m_devicehandler; /**< The device handler used for command buffer
                            operations. */
    std::shared_ptr<swap_chain::SwapChain>
        m_swapChain; /**< The swap chain associated with the command buffers. */
    std::shared_ptr<graphics_pipeline::AbstractGraphicsPipeline>
        m_graphicsPipeline; /**< The graphics pipeline associated with the
                               command buffers. */

    /**
     * \brief Creates the Vulkan command pool.
     *
     * This function creates the Vulkan command pool for allocating command
     * buffers.
     */
    void m_createCommandPool();
};

} // namespace command_buffer
