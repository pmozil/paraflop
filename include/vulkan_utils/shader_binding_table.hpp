#pragma once
#include "common.hpp"
#include "vulkan_utils/device.hpp"

namespace raytracer {
class ShaderBindingTable {
  public:
    VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};
    VkDevice device;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor;
    VkDeviceSize size = 0;
    VkDeviceSize alignment = 0;
    void *mapped = nullptr;
    VkResult create(std::shared_ptr<device::DeviceHandler> &deviceHandler,
                    VkBufferUsageFlags usageFlags,
                    VkMemoryPropertyFlags memoryPropertyFlags,
                    VkDeviceSize size, void *data = nullptr);
    /** @brief Usage flags to be filled by external source at buffer
     * creation (to query at some later point) */
    VkBufferUsageFlags usageFlags;
    /** @brief Memory property flags to be filled by external source at
     * buffer creation (to query at some later point) */
    VkMemoryPropertyFlags memoryPropertyFlags;
    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();
    [[nodiscard]] VkResult bind(VkDeviceSize offset = 0) const;
    void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE,
                         VkDeviceSize offset = 0);
    [[nodiscard]] VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE,
                                 VkDeviceSize offset = 0) const;
    [[nodiscard]] VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE,
                                      VkDeviceSize offset = 0) const;
    void destroy() const;
};
} // namespace raytracer
