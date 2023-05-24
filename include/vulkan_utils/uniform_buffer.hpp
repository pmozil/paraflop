#pragma once
#include "common.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace buffer {
class UniformBuffer : public Buffer {
  public:
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
    ~UniformBuffer() { destroy(); }
};
} // namespace buffer
