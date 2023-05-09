#pragma once
#include "command_buffer.hpp"
#include "common.hpp"
#include "device.hpp"

namespace buffer {
class Buffer {
  public:
    Buffer(device::DeviceHandler *deviceHandler,
           command_buffer::CommandBufferHandler *commandBuffer,
           VkBufferUsageFlags usageFlags,
           VkMemoryPropertyFlags memoryPropertyFlags);
    void map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();
    void bind(VkDeviceSize offset = 0);
    void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE,
                         VkDeviceSize offset = 0);
    void copyTo(void *data, VkDeviceSize size);
    void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void destroy();

    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor;
    VkDeviceSize size = 0;
    VkDeviceSize alignment = 0;
    void *mapped = nullptr;
    VkBufferUsageFlags usageFlags;
    VkMemoryPropertyFlags memoryPropertyFlags;

  private:
    command_buffer::CommandBufferHandler *commandBuffer;
    device::DeviceHandler *deviceHandler;
    void createBuffer();
    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);
};
} // namespace buffer
