#pragma once
#include "common.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace buffer {
/**
 * \class UniformBuffer
 *
 * \brief Represents a uniform buffer used for storing uniform data in Vulkan.
 *
 * The UniformBuffer class is a specialized type of Buffer that is used for
 * storing uniform data in Vulkan. It provides convenience functions for
 * creating and managing uniform buffers with specific usage and memory
 * properties.
 */
class UniformBuffer : public Buffer {
  public:
    /**
     * \fn UniformBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler,
     *                   std::shared_ptr<command_buffer::CommandBufferHandler>
     * commandBuffer, VkDeviceSize size)
     *
     * \brief Constructs a UniformBuffer object.
     *
     * \param deviceHandler The device handler associated with the uniform
     * buffer.
     * \param commandBuffer The command buffer associated with the uniform
     * buffer.
     * \param size The size of the uniform buffer.
     */
    UniformBuffer(
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer,
        VkDeviceSize size)
        : Buffer(std::move(deviceHandler), std::move(commandBuffer),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 VK_SHARING_MODE_EXCLUSIVE, size){};

    UniformBuffer() = default;

    /**
     * \fn UniformBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler,
     *                   std::shared_ptr<command_buffer::CommandBufferHandler>
     * commandBuffer, VkDeviceSize size, VkSharingMode mode)
     *
     * \brief Constructs a UniformBuffer object with a specified sharing mode.
     *
     * \param deviceHandler The device handler associated with the uniform
     * buffer.
     * \param commandBuffer The command buffer associated with the
     * uniform buffer.
     * \param size The size of the uniform buffer.
     * \param mode The sharing mode of the uniform buffer.
     */
    UniformBuffer(
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer,
        VkDeviceSize size, VkSharingMode mode)
        : Buffer(std::move(deviceHandler), std::move(commandBuffer),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 mode, size){};

    /**
     * \fn ~UniformBuffer()
     *
     * \brief Destructor for the UniformBuffer class.
     */
    ~UniformBuffer() { destroy(); }
};

} // namespace buffer
