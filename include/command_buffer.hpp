#pragma once
#include "common.hpp"
#include "device.hpp"

namespace command_buffer {
class CommandBuffer {
  public:
    CommandBuffer(device::DeviceHandler *deviceHandler)
        : deviceHandler(deviceHandler) {
        createCommandPool();
        createCommandBuffer();
    }
    inline VkCommandBuffer &getCommandBuffer() { return commandBuffer; }

  private:
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    device::DeviceHandler *deviceHandler;
    void createCommandPool();
    void createCommandBuffer();
};
} // namespace command_buffer
