#pragma once
#include "common.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace buffer {
/**
 * \class IndexBuffer
 *
 * \brief Represents an index buffer used for storing index data in Vulkan.
 *
 * The IndexBuffer class is a specialized type of Buffer that is used for
 * storing index data in Vulkan. It provides convenience functions for creating
 * and managing index buffers with specific usage and memory properties.
 */
class IndexBuffer : public Buffer {
  public:
    /**
     * \fn IndexBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler,
     *                std::shared_ptr<command_buffer::CommandBufferHandler>
     *                commandBuffer, VkDeviceSize size)
     *
     * \brief Constructs an IndexBuffer
     * object.
     *
     * \param deviceHandler The device handler associated with the index buffer.
     * \param commandBuffer The command buffer associated with the index buffer.
     * \param size The size of the index buffer.
     */
    IndexBuffer(
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer,
        VkDeviceSize size)
        : Buffer(std::move(deviceHandler), std::move(commandBuffer),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE,
                 size){};

    /**
     * \fn IndexBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler,
     *                std::shared_ptr<command_buffer::CommandBufferHandler>
     * commandBuffer, VkDeviceSize size, VkSharingMode mode)
     *
     * \brief Constructs
     * an IndexBuffer object with a specified sharing mode.
     *
     * \param deviceHandler The device handler associated with the index buffer.
     * \param commandBuffer The command buffer associated with the index buffer.
     * \param size The size of the index buffer.
     * \param mode The sharing mode of the index buffer.
     */
    IndexBuffer(
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer,
        VkDeviceSize size, VkSharingMode mode)
        : Buffer(std::move(deviceHandler), std::move(commandBuffer),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mode, size){};

    /**
     * \fn ~IndexBuffer()
     *
     * \brief Destructor for the IndexBuffer class.
     */
    ~IndexBuffer() { destroy(); }
};

} // namespace buffer
