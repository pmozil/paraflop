#pragma once
#include "common.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace buffer {
/**
 * \class StagingBuffer
 *
 * \brief Represents a staging buffer used for CPU-to-GPU data transfer.
 *
 * The StagingBuffer class inherits from the Buffer class and provides
 * functionality for creating and managing staging buffers in Vulkan. Staging
 * buffers are used to transfer data from the CPU to the GPU. This class
 * supports different constructors to handle various scenarios of staging buffer
 * creation.
 */
class StagingBuffer : public Buffer {
  public:
    /**
     * \fn StagingBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler,
     *                  std::shared_ptr<command_buffer::CommandBufferHandler>
     * commandBuffer, VkDeviceSize size) \brief Constructs a StagingBuffer
     * object.
     *
     * \param deviceHandler The device handler associated with the staging
     * buffer.
     * \param commandBuffer The command buffer handler associated with
     * the staging buffer.
     * \param size The size of the staging buffer.
     */
    StagingBuffer(
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer,
        VkDeviceSize size)
        : Buffer(std::move(deviceHandler), std::move(commandBuffer),
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 VK_SHARING_MODE_EXCLUSIVE, size){};

    /**
     * \fn StagingBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler,
     *                  std::shared_ptr<command_buffer::CommandBufferHandler>
     * commandBuffer, VkDeviceSize size, VkSharingMode mode) \brief Constructs a
     * StagingBuffer object with a specified sharing mode.
     *
     * \param deviceHandler The device handler associated with the staging
     * buffer.
     * \param commandBuffer The command buffer handler associated with
     * the staging buffer.
     * \param size The size of the staging buffer.
     * \param mode The sharing mode for the staging buffer.
     */
    StagingBuffer(
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer,
        VkDeviceSize size, VkSharingMode mode)
        : Buffer(std::move(deviceHandler), std::move(commandBuffer),
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 mode, size){};

    /**
     * \fn ~StagingBuffer()
     *
     * \brief Destructor for the StagingBuffer class.
     */
    ~StagingBuffer() { destroy(); }
};

} // namespace buffer
