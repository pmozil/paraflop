#include "buffer.hpp"
#include "create_info.hpp"

namespace buffer {
Buffer::Buffer(device::DeviceHandler *deviceHandler,
               command_buffer::CommandBufferHandler *commandBuffer,
               VkBufferUsageFlags usageFlags,
               VkMemoryPropertyFlags memoryPropertyFlags)
    : usageFlags(usageFlags), memoryPropertyFlags(memoryPropertyFlags),
      commandBuffer(commandBuffer), deviceHandler(deviceHandler) {
    createBuffer();
}

void Buffer::createBuffer() {
    VkBufferCreateInfo bufferInfo =
        create_info::bufferCreateInfo(size, usageFlags);

    if (vkCreateBuffer(deviceHandler->logicalDevice, &bufferInfo, nullptr,
                       &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(deviceHandler->logicalDevice, buffer,
                                  &memRequirements);

    VkMemoryAllocateInfo allocInfo = create_info::memoryAllocInfo(
        size,
        findMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags));

    if (vkAllocateMemory(deviceHandler->logicalDevice, &allocInfo, nullptr,
                         &memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(deviceHandler->logicalDevice, buffer, memory, 0);
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter,
                                VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(deviceHandler->physicalDevice,
                                        &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
    VK_CHECK(vkMapMemory(deviceHandler->logicalDevice, memory, offset, size, 0,
                         &mapped));
}

void Buffer::unmap() {
    if (mapped != nullptr) {
        vkUnmapMemory(deviceHandler->logicalDevice, memory);
        mapped = nullptr;
    }
}

void Buffer::bind(VkDeviceSize offset) {
    VK_CHECK(vkBindBufferMemory(deviceHandler->logicalDevice, buffer, memory,
                                offset));
}

void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset) {
    descriptor.offset = offset;
    descriptor.buffer = buffer;
    descriptor.range = size;
}

void Buffer::copyTo(void *data, VkDeviceSize size) {
    assert(mapped);
    memcpy(mapped, data, size);
}

void Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    VK_CHECK(vkFlushMappedMemoryRanges(deviceHandler->logicalDevice, 1,
                                       &mappedRange));
}

void Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    VK_CHECK(vkInvalidateMappedMemoryRanges(deviceHandler->logicalDevice, 1,
                                            &mappedRange));
}

void Buffer::destroy() {
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(deviceHandler->logicalDevice, buffer, nullptr);
    }
    if (memory != nullptr) {
        vkFreeMemory(deviceHandler->logicalDevice, memory, nullptr);
    }
}
void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                        VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo =
        create_info::commandBuffferAllocInfo(commandBuffer->commandPool, 1);

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(deviceHandler->logicalDevice, &allocInfo,
                             &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo = create_info::commabdBufferBeginInfo();

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferCopy copyRegion = create_info::copyRegion(size);
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo = create_info::submitInfo(1, &cmdBuffer);

    vkQueueSubmit(deviceHandler->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(deviceHandler->graphicsQueue);

    vkFreeCommandBuffers(deviceHandler->logicalDevice,
                         commandBuffer->commandPool, 1, &cmdBuffer);
}
} // namespace buffer
