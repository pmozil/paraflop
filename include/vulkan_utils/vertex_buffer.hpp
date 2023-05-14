#pragma once
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/common.hpp"
#include "vulkan_utils/device.hpp"

namespace buffer {
class VertexBuffer : public Buffer {
  public:
    VertexBuffer(device::DeviceHandler *DeviceHandler,
                 command_buffer::CommandBufferHandler *commandBuffer);

  private:
};
} // namespace buffer
