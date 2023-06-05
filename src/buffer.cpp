#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/create_info.hpp"

namespace buffer {
Buffer::Buffer(
    std::shared_ptr<device::DeviceHandler> m_deviceHandler,
    std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer,
    VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
    VkSharingMode sharingMode, VkDeviceSize size)
    : size(size), usageFlags(usageFlags),
      memoryPropertyFlags(memoryPropertyFlags),
      m_commandBuffer(std::move(m_commandBuffer)),
      m_deviceHandler(std::move(m_deviceHandler)) {
    m_makeBuffer(size, usageFlags, memoryPropertyFlags, buffer, memory,
                 sharingMode);
}

void Buffer::m_makeBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkBuffer &buffer,
                          VkDeviceMemory &bufferMemory,
                          VkSharingMode sharingMode) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;

    VK_CHECK(vkCreateBuffer(*m_deviceHandler, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*m_deviceHandler, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_deviceHandler->getMemoryType(
        memRequirements.memoryTypeBits, properties);

    VK_CHECK(
        vkAllocateMemory(*m_deviceHandler, &allocInfo, nullptr, &bufferMemory));

    vkBindBufferMemory(*m_deviceHandler, buffer, bufferMemory, 0);
}

void Buffer::map() {
    VK_CHECK(vkMapMemory(*m_deviceHandler, memory, 0, size, 0, &mapped));
}

void Buffer::unmap() {
    if (mapped != nullptr) {
        vkUnmapMemory(*m_deviceHandler, memory);
        mapped = nullptr;
    }
}

void Buffer::bind(VkDeviceSize offset) {
    VK_CHECK(vkBindBufferMemory(*m_deviceHandler, buffer, memory, offset));
}

void Buffer::setupDescriptor() {
    descriptor.offset = 0;
    descriptor.buffer = buffer;
    descriptor.range = size;
}

void Buffer::copy(void *data, VkDeviceSize size) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    m_makeBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory, VK_SHARING_MODE_EXCLUSIVE);

    void *mem;
    vkMapMemory(*m_deviceHandler, stagingBufferMemory, 0, size, 0, &mem);
    memcpy(mem, data, (size_t)size);
    vkUnmapMemory(*m_deviceHandler, stagingBufferMemory);

    copyFrom(stagingBuffer);

    vkDestroyBuffer(*m_deviceHandler, stagingBuffer, nullptr);
    vkFreeMemory(*m_deviceHandler, stagingBufferMemory, nullptr);
}

void Buffer::fastCopy(void *data, VkDeviceSize size) {
    if (mapped == nullptr) {
        map();
    }
    memcpy(mapped, data, (size_t)size);
}

void Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    VK_CHECK(vkFlushMappedMemoryRanges(*m_deviceHandler, 1, &mappedRange));
}

void Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    VK_CHECK(vkInvalidateMappedMemoryRanges(*m_deviceHandler, 1, &mappedRange));
}

void Buffer::destroy() {
    if (mapped != nullptr) {
        unmap();
    }
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(*m_deviceHandler, buffer, nullptr);
    }
    if (memory != nullptr) {
        vkFreeMemory(*m_deviceHandler, memory, nullptr);
    }
}

void Buffer::copyFrom(VkBuffer srcBuffer) {
    VkCommandBufferAllocateInfo allocInfo =
        create_info::commandBufferAllocInfo(m_commandBuffer->commandPool, 1);

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(*m_deviceHandler, &allocInfo, &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo = create_info::commandBufferBeginInfo();

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferCopy copyRegion = create_info::copyRegion(size);
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, buffer, 1, &copyRegion);

    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo = create_info::submitInfo(1, &cmdBuffer);

    VkQueue transferQueue = m_deviceHandler->getTransferQueue();
    vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(transferQueue);

    vkFreeCommandBuffers(*m_deviceHandler, m_commandBuffer->commandPool, 1,
                         &cmdBuffer);
}

void Buffer::copyTo(VkBuffer dstBuffer) {
    VkCommandBufferBeginInfo beginInfo = create_info::commandBufferBeginInfo();

    vkBeginCommandBuffer(m_commandBuffer->transferBuffer, &beginInfo);

    VkBufferCopy copyRegion = create_info::copyRegion(size);
    vkCmdCopyBuffer(m_commandBuffer->transferBuffer, buffer, dstBuffer, 1,
                    &copyRegion);

    vkEndCommandBuffer(m_commandBuffer->transferBuffer);

    VkSubmitInfo submitInfo =
        create_info::submitInfo(1, &m_commandBuffer->transferBuffer);

    VkQueue transferQueue = m_deviceHandler->getTransferQueue();
    vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(transferQueue);

    vkFreeCommandBuffers(*m_deviceHandler, m_commandBuffer->commandPool, 1,
                         &m_commandBuffer->transferBuffer);
}
} // namespace buffer
