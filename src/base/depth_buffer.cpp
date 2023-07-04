#include "vulkan_utils/depth_buffer.hpp"
#include "vulkan_utils/create_info.hpp"

namespace swap_chain {
DepthBuffer::DepthBuffer(std::shared_ptr<device::DeviceHandler> deviceHandler)
    : m_deviceHandler(std::move(deviceHandler)) {}

VkFormat DepthBuffer::m_findSupportedFormat(VkImageTiling tiling,
                                            VkFormatFeatureFlags features) {
    for (VkFormat format : depth_formats) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_deviceHandler->physicalDevice,
                                            format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        }

        if (tiling == VK_IMAGE_TILING_OPTIMAL &&
            (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

void DepthBuffer::createDepthResources(uint32_t width, uint32_t height) {
    format =
        m_findSupportedFormat(VK_IMAGE_TILING_OPTIMAL,
                              VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkImageCreateInfo imgCreateInfo = create_info::imageCreateInfo(
        VK_IMAGE_TYPE_2D, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    imgCreateInfo.extent.width = width;
    imgCreateInfo.extent.height = height;

    VK_CHECK(
        vkCreateImage(*m_deviceHandler, &imgCreateInfo, nullptr, &depthImage));

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(*m_deviceHandler, depthImage, &memReqs);

    VkMemoryAllocateInfo allocInfo = create_info::memoryAllocInfo(
        memReqs.size,
        m_deviceHandler->getMemoryType(memReqs.memoryTypeBits,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &allocInfo, nullptr,
                              &depthImageMemory));

    vkBindImageMemory(*m_deviceHandler, depthImage, depthImageMemory, 0);

    VkImageViewCreateInfo viewInfo =
        create_info::imageViewCreateInfo(depthImage, format);
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    VK_CHECK(vkCreateImageView(*m_deviceHandler, &viewInfo, nullptr,
                               &depthImageView));
}

void DepthBuffer::cleanup() {
    vkDestroyImageView(*m_deviceHandler, depthImageView, nullptr);
    vkDestroyImage(*m_deviceHandler, depthImage, nullptr);
    vkFreeMemory(*m_deviceHandler, depthImageMemory, nullptr);
}
} // namespace swap_chain
