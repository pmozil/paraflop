#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/create_info.hpp"

namespace buffer {
Buffer::Buffer(device::DeviceHandler *m_devicehandler,
               command_buffer::CommandBufferHandler *m_commandBuffer,
               VkBufferUsageFlags usageFlags,
               VkMemoryPropertyFlags memoryPropertyFlags,
               VkSharingMode sharingMode)
    : usageFlags(usageFlags), memoryPropertyFlags(memoryPropertyFlags),
      m_commandBuffer(m_commandBuffer), m_devicehandler(m_devicehandler) {
    m_createBuffer(sharingMode);
}

void Buffer::m_createBuffer(VkSharingMode sharingMode) {
    VkBufferCreateInfo bufferInfo =
        create_info::bufferCreateInfo(size, usageFlags, sharingMode);

    if (vkCreateBuffer(m_devicehandler->logicalDevice, &bufferInfo, nullptr,
                       &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_devicehandler->logicalDevice, buffer,
                                  &memRequirements);

    VkMemoryAllocateInfo allocInfo = create_info::memoryAllocInfo(
        size,
        m_findMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags));

    if (vkAllocateMemory(m_devicehandler->logicalDevice, &allocInfo, nullptr,
                         &memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_devicehandler->logicalDevice, buffer, memory, 0);
}

uint32_t Buffer::m_findMemoryType(uint32_t typeFilter,
                                  VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_devicehandler->physicalDevice,
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
    VK_CHECK(vkMapMemory(m_devicehandler->logicalDevice, memory, offset, size,
                         0, &mapped));
}

void Buffer::unmap() {
    if (mapped != nullptr) {
        vkUnmapMemory(m_devicehandler->logicalDevice, memory);
        mapped = nullptr;
    }
}

void Buffer::bind(VkDeviceSize offset) {
    VK_CHECK(vkBindBufferMemory(m_devicehandler->logicalDevice, buffer, memory,
                                offset));
}

void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset) {
    descriptor.offset = offset;
    descriptor.buffer = buffer;
    descriptor.range = size;
}

void Buffer::copy(void *data, VkDeviceSize size) const {
    assert(mapped);
    memcpy(mapped, data, size);
}

void Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    VK_CHECK(vkFlushMappedMemoryRanges(m_devicehandler->logicalDevice, 1,
                                       &mappedRange));
}

void Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    VK_CHECK(vkInvalidateMappedMemoryRanges(m_devicehandler->logicalDevice, 1,
                                            &mappedRange));
}

void Buffer::destroy() {
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_devicehandler->logicalDevice, buffer, nullptr);
    }
    if (memory != nullptr) {
        vkFreeMemory(m_devicehandler->logicalDevice, memory, nullptr);
    }
}

void Buffer::copyFrom(VkBuffer srcBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo =
        create_info::commandBuffferAllocInfo(m_commandBuffer->commandPool, 1);

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(m_devicehandler->logicalDevice, &allocInfo,
                             &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo = create_info::commabdBufferBeginInfo();

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferCopy copyRegion = create_info::copyRegion(size);
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, buffer, 1, &copyRegion);

    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo = create_info::submitInfo(1, &cmdBuffer);

    VkQueue transferQueue = m_devicehandler->getTransferQueue();
    vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(transferQueue);

    vkFreeCommandBuffers(m_devicehandler->logicalDevice,
                         m_commandBuffer->commandPool, 1, &cmdBuffer);
}

void Buffer::copyTo(VkBuffer dstBuffer, VkDeviceSize size) {
    // VkCommandBufferAllocateInfo allocInfo =
    //     create_info::commandBuffferAllocInfo(m_commandBuffer->commandPool,
    //     1);

    // VkCommandBuffer cmdBuffer;
    // vkAllocateCommandBuffers(m_devicehandler->logicalDevice, &allocInfo,
    //                          &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo = create_info::commabdBufferBeginInfo();

    vkBeginCommandBuffer(m_commandBuffer->transferBuffer, &beginInfo);

    VkBufferCopy copyRegion = create_info::copyRegion(size);
    vkCmdCopyBuffer(m_commandBuffer->transferBuffer, buffer, dstBuffer, 1,
                    &copyRegion);

    vkEndCommandBuffer(m_commandBuffer->transferBuffer);

    VkSubmitInfo submitInfo =
        create_info::submitInfo(1, &m_commandBuffer->transferBuffer);

    VkQueue transferQueue = m_devicehandler->getTransferQueue();
    vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(transferQueue);

    vkFreeCommandBuffers(m_devicehandler->logicalDevice,
                         m_commandBuffer->commandPool, 1,
                         &m_commandBuffer->transferBuffer);
}
} // namespace buffer
