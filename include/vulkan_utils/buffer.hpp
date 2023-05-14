#pragma once
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/common.hpp"
#include "vulkan_utils/device.hpp"

namespace buffer {
class Buffer {
  public:
    Buffer(device::DeviceHandler *m_devicehandler,
           command_buffer::CommandBufferHandler *m_commandBuffer,
           VkBufferUsageFlags usageFlags,
           VkMemoryPropertyFlags memoryPropertyFlags,
           VkSharingMode sharingMode);
    void map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();
    void bind(VkDeviceSize offset = 0);
    void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE,
                         VkDeviceSize offset = 0);
    void copy(void *data, VkDeviceSize size) const;
    void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void copyFrom(VkBuffer srcBuffer, VkDeviceSize size);
    void copyTo(VkBuffer dstBuffer, VkDeviceSize size);
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
    command_buffer::CommandBufferHandler *m_commandBuffer;
    device::DeviceHandler *m_devicehandler;
    void m_createBuffer(VkSharingMode sharingMode);
    uint32_t m_findMemoryType(uint32_t typeFilter,
                              VkMemoryPropertyFlags properties);
};
} // namespace buffer
