#pragma once
#include "common.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace buffer {
class VertexBuffer : public Buffer {
  public:
    VertexBuffer(
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer,
        VkDeviceSize size)
        : Buffer(std::move(deviceHandler), std::move(commandBuffer),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE,
                 size){};
    VertexBuffer(
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer,
        VkDeviceSize size, VkSharingMode mode)
        : Buffer(std::move(deviceHandler), std::move(commandBuffer),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mode, size){};
    ~VertexBuffer() { destroy(); }
};
} // namespace buffer
