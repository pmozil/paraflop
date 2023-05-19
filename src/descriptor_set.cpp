#include "vulkan_utils/descriptor_set.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/device.hpp"

namespace descriptor_set {
DescriptorSetLayout::DescriptorSetLayout(
    std::shared_ptr<device::DeviceHandler> m_deviceHandler,
    VkDescriptorSetLayoutBinding *bindings)
    : m_deviceHandler(std::move(m_deviceHandler)) {
    createLayout(bindings);
}

void DescriptorSetLayout::createLayout(VkDescriptorSetLayoutBinding *bindings) {
    VkDescriptorSetLayoutCreateInfo createInfo =
        create_info::descriptorSetLayoutInfo(bindings);

    VK_CHECK(vkCreateDescriptorSetLayout(m_deviceHandler->logicalDevice,
                                         &createInfo, nullptr, &layout));
}
} // namespace descriptor_set
