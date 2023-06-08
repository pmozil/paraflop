#pragma once

#include "common.hpp"
#include "vulkan_utils/device.hpp"

namespace swap_chain {
static const std::array<VkFormat, 3> depth_formats = {
    VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT};

class DepthBuffer {
  public:
    DepthBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler);
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    VkFormat format;
    void createDepthResources(uint32_t width, uint32_t height);
    void cleanup();

  private:
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    VkFormat m_findSupportedFormat(VkImageTiling tiling,
                                   VkFormatFeatureFlags features);
};
} // namespace swap_chain
