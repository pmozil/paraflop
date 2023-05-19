#include "vulkan_utils/descriptor_set.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/device.hpp"

namespace descriptor_set {
DescriptorSetLayout::DescriptorSetLayout(
    std::shared_ptr<device::DeviceHandler> m_deviceHandler,
    VkDescriptorType type, VkShaderStageFlags flags)
    : m_deviceHandler(std::move(m_deviceHandler)) {
    createLayout(type, flags);
}

void DescriptorSetLayout::createLayout(VkDescriptorType type,
                                       VkShaderStageFlags flags) {
    VkDescriptorSetLayoutCreateInfo createInfo =
        create_info::descriptorSetLayoutInfo(type, flags);

    VK_CHECK(vkCreateDescriptorSetLayout(m_deviceHandler->logicalDevice,
                                         &createInfo, nullptr, &layout));
}
} // namespace descriptor_set
