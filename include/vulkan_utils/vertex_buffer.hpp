#pragma once
#include "common.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace buffer {
/**
 * \class VertexBuffer
 *
 * \brief Represents a vertex buffer used for vertex data storage.
 *
 * The VertexBuffer class inherits from the Buffer class and provides
 * functionality for creating and managing vertex buffers in Vulkan. It supports
 * different constructors to handle various scenarios of vertex buffer creation.
 */
class VertexBuffer : public Buffer {
  public:
    /**
     * \fn VertexBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler,
     *                  std::shared_ptr<command_buffer::CommandBufferHandler>
     * commandBuffer, VkDeviceSize size) \brief Constructs a VertexBuffer
     * object.
     *
     * \param deviceHandler The device handler associated with the vertex
     * buffer.
     * \param commandBuffer The command buffer handler associated with
     * the vertex buffer.
     * \param size The size of the vertex buffer.
     */
    VertexBuffer(
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer,
        VkDeviceSize size)
        : Buffer(std::move(deviceHandler), std::move(commandBuffer),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE,
                 size){};

    /**
     * \fn VertexBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler,
     *                  std::shared_ptr<command_buffer::CommandBufferHandler>
     * commandBuffer, VkDeviceSize size, VkSharingMode mode)
     *
     * \brief Constructs a
     * VertexBuffer object with a specified sharing mode.
     *
     * \param deviceHandler The device handler associated with the vertex
     * buffer.
     * \param commandBuffer The command buffer handler associated with
     * the vertex buffer.
     * \param size The size of the vertex buffer.
     * \param mode The sharing mode for the vertex buffer.
     */
    VertexBuffer(
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer,
        VkDeviceSize size, VkSharingMode mode)
        : Buffer(std::move(deviceHandler), std::move(commandBuffer),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mode, size){};

    /**
     * \fn ~VertexBuffer()
     *
     * \brief Destructor for the VertexBuffer class.
     */
    ~VertexBuffer() { destroy(); }
};

} // namespace buffer
