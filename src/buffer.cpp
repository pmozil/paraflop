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
    m_createBuffer(sharingMode);
}

void Buffer::m_createBuffer(VkSharingMode sharingMode) {
    VkBufferCreateInfo bufferInfo =
        create_info::bufferCreateInfo(size, usageFlags, sharingMode);

    VK_CHECK(vkCreateBuffer(m_deviceHandler->logicalDevice, &bufferInfo,
                            nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_deviceHandler->logicalDevice, buffer,
                                  &memRequirements);

    VkMemoryAllocateInfo allocInfo = create_info::memoryAllocInfo(
        size,
        m_findMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags));

    VK_CHECK(vkAllocateMemory(m_deviceHandler->logicalDevice, &allocInfo,
                              nullptr, &memory));

    vkBindBufferMemory(m_deviceHandler->logicalDevice, buffer, memory, 0);
}

uint32_t Buffer::m_findMemoryType(uint32_t typeFilter,
                                  VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_deviceHandler->physicalDevice,
                                        &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (static_cast<bool>(typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
    VK_CHECK(vkMapMemory(m_deviceHandler->logicalDevice, memory, offset, size,
                         0, &mapped));
}

void Buffer::unmap() {
    if (mapped != nullptr) {
        vkUnmapMemory(m_deviceHandler->logicalDevice, memory);
        mapped = nullptr;
    }
}

void Buffer::bind(VkDeviceSize offset) {
    VK_CHECK(vkBindBufferMemory(m_deviceHandler->logicalDevice, buffer, memory,
                                offset));
}

void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset) {
    descriptor.offset = offset;
    descriptor.buffer = buffer;
    descriptor.range = size;
}

void Buffer::copy(void *data, VkDeviceSize size) const {
    assert(mapped != nullptr);
    memcpy(mapped, data, size);
}

void Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    VK_CHECK(vkFlushMappedMemoryRanges(m_deviceHandler->logicalDevice, 1,
                                       &mappedRange));
}

void Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    VK_CHECK(vkInvalidateMappedMemoryRanges(m_deviceHandler->logicalDevice, 1,
                                            &mappedRange));
}

void Buffer::destroy() {
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_deviceHandler->logicalDevice, buffer, nullptr);
    }
    if (memory != nullptr) {
        vkFreeMemory(m_deviceHandler->logicalDevice, memory, nullptr);
    }
}

void Buffer::copyFrom(VkBuffer srcBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo =
        create_info::commandBuffferAllocInfo(m_commandBuffer->commandPool, 1);

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(m_deviceHandler->logicalDevice, &allocInfo,
                             &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo = create_info::commabdBufferBeginInfo();

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferCopy copyRegion = create_info::copyRegion(size);
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, buffer, 1, &copyRegion);

    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo = create_info::submitInfo(1, &cmdBuffer);

    VkQueue transferQueue = m_deviceHandler->getTransferQueue();
    vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(transferQueue);

    vkFreeCommandBuffers(m_deviceHandler->logicalDevice,
                         m_commandBuffer->commandPool, 1, &cmdBuffer);
}

void Buffer::copyTo(VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferBeginInfo beginInfo = create_info::commabdBufferBeginInfo();

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

    vkFreeCommandBuffers(m_deviceHandler->logicalDevice,
                         m_commandBuffer->commandPool, 1,
                         &m_commandBuffer->transferBuffer);
}
} // namespace buffer
