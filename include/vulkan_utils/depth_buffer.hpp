#pragma once

#include "common.hpp"
#include "vulkan_utils/device.hpp"

namespace swap_chain {
static const std::array<VkFormat, 3> depth_formats = {
    VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT};

/**
 * @brief The DepthBuffer class represents a depth buffer used for rendering.
 */
class DepthBuffer {
  public:
    /**
     * @brief Constructs a DepthBuffer object.
     * @param deviceHandler The shared pointer to the device handler.
     */
    DepthBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler);

    VkImage depthImage; /**< The depth image. */
    VkDeviceMemory
        depthImageMemory;       /**< The device memory for the depth image. */
    VkImageView depthImageView; /**< The image view for the depth image. */
    VkFormat format;            /**< The format of the depth buffer. */

    /**
     * @brief Creates the depth resources.
     * @param width The width of the depth buffer.
     * @param height The height of the depth buffer.
     */
    void createDepthResources(uint32_t width, uint32_t height);

    /**
     * @brief Cleans up the depth resources.
     */
    void cleanup();

  private:
    std::shared_ptr<device::DeviceHandler>
        m_deviceHandler; /**< The shared pointer to the device handler. */

    /**
     * @brief Finds a supported format for the depth buffer.
     * @param tiling The tiling mode of the image.
     * @param features The required format features.
     * @return The supported format for the depth buffer.
     */
    VkFormat m_findSupportedFormat(VkImageTiling tiling,
                                   VkFormatFeatureFlags features);
};

} // namespace swap_chain
