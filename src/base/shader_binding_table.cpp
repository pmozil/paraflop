#include "vulkan_utils/shader_binding_table.hpp"
#include "vulkan_utils/create_info.hpp"

namespace raytracer {

VkResult ShaderBindingTable::map(VkDeviceSize size, VkDeviceSize offset) {
    return vkMapMemory(device, memory, offset, size, 0, &mapped);
}

void ShaderBindingTable::unmap() {
    if (mapped != nullptr) {
        vkUnmapMemory(device, memory);
        mapped = nullptr;
    }
}

[[nodiscard]] VkResult ShaderBindingTable::bind(VkDeviceSize offset) const {
    return vkBindBufferMemory(device, buffer, memory, offset);
}

void ShaderBindingTable::setupDescriptor(VkDeviceSize size,
                                         VkDeviceSize offset) {
    descriptor.offset = offset;
    descriptor.buffer = buffer;
    descriptor.range = size;
}

[[nodiscard]] VkResult ShaderBindingTable::flush(VkDeviceSize size,
                                                 VkDeviceSize offset) const {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
}

[[nodiscard]] VkResult
ShaderBindingTable::invalidate(VkDeviceSize size, VkDeviceSize offset) const {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
}

void ShaderBindingTable::destroy() const {
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, buffer, nullptr);
    }
    if (memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, memory, nullptr);
    }
}

VkResult ShaderBindingTable::create(
    std::shared_ptr<device::DeviceHandler> &deviceHandler,
    VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize size, void *data) {
    this->device = *deviceHandler;

    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo =
        create_info::bufferCreateInfo(usageFlags, size);
    VK_CHECK(vkCreateBuffer(this->device, &bufferCreateInfo, nullptr, &buffer));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = create_info::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(*deviceHandler, buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = deviceHandler->getMemoryType(
        memReqs.memoryTypeBits, memoryPropertyFlags);
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also
    // need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (static_cast<bool>(usageFlags &
                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    VK_CHECK(vkAllocateMemory(*deviceHandler, &memAlloc, nullptr, &memory));

    this->alignment = memReqs.alignment;
    this->size = size;
    this->usageFlags = usageFlags;
    this->memoryPropertyFlags = memoryPropertyFlags;

    // If a pointer to the buffer data has been passed, map the buffer and copy
    // over the data
    if (data != nullptr) {
        VK_CHECK(map());
        memcpy(mapped, data, size);
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VK_CHECK(flush());
        }

        unmap();
    }

    // Initialize a default descriptor that covers the whole buffer size
    setupDescriptor();

    // Attach the memory to the buffer object
    return bind();
}
} // namespace raytracer
