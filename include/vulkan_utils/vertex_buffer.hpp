#pragma once
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/common.hpp"
#include "vulkan_utils/device.hpp"

namespace buffer {
class StagingBuffer : public Buffer {
  public:
    StagingBuffer(device::DeviceHandler *deviceHandler,
                  command_buffer::CommandBufferHandler *commandBuffer)
        : Buffer(deviceHandler, commandBuffer,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 VK_SHARING_MODE_CONCURRENT){};
};
} // namespace buffer
