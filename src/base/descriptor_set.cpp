#include "vulkan_utils/descriptor_set.hpp"

namespace descriptor_set {
DescriptorSetLayout::DescriptorSetLayout(
    std::shared_ptr<device::DeviceHandler> m_deviceHandler,
    VkDescriptorSetLayoutBinding *bindings, uint32_t bind_count)
    : m_deviceHandler(std::move(m_deviceHandler)) {
    m_createLayout(bindings, bind_count);
}

void DescriptorSetLayout::m_createLayout(VkDescriptorSetLayoutBinding *bindings,
                                         uint32_t bind_count) {
    VkDescriptorSetLayoutCreateInfo createInfo =
        create_info::descriptorSetLayoutInfo(bindings, bind_count);

    VK_CHECK(vkCreateDescriptorSetLayout(*m_deviceHandler, &createInfo, nullptr,
                                         &layout));
}

void DescriptorSetLayout::cleanup() {
    vkDestroyDescriptorSetLayout(*m_deviceHandler, layout, nullptr);
}

DescriptorSetHandler::DescriptorSetHandler(
    std::shared_ptr<device::DeviceHandler> m_deviceHandler,
    std::shared_ptr<DescriptorSetLayout> m_layout,
    std::vector<VkDescriptorPoolSize> sizes,
    std::vector<VkWriteDescriptorSet> writes)
    : sizes(std::move(sizes)), writes(std::move(writes)),
      m_deviceHandler(std::move(m_deviceHandler)),
      m_layout(std::move(m_layout)) {
    m_createDescriptorPool();
    m_createDescriptorSets();
}

void DescriptorSetHandler::m_createDescriptorPool() {
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
    poolInfo.pPoolSizes = sizes.data();
    poolInfo.maxSets = 1;

    VK_CHECK(vkCreateDescriptorPool(*m_deviceHandler, &poolInfo, nullptr,
                                    &descriptorPool));
}

void DescriptorSetHandler::m_createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *m_layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts.data();

    VK_CHECK(
        vkAllocateDescriptorSets(*m_deviceHandler, &allocInfo, &descriptorSet));

    // for (int i = 0; i < writes.size(); i++) {
    //     writes[i].dstSet = descriptorSet;
    //     std::cout << "DEBUG: " << writes[0].dstSet << "\n";
    //     std::cout << "DEBUG: " << descriptorSet << "\n";
    // }

    for (auto &write : writes) {
        write.dstSet = descriptorSet;
        std::cout << "DEBUG: " << write.dstSet << "\n";
        std::cout << "DEBUG: " << descriptorSet << "\n";
    }

    vkUpdateDescriptorSets(*m_deviceHandler,
                           static_cast<uint32_t>(writes.size()), writes.data(),
                           0, nullptr);
}

void DescriptorSetHandler::cleanup() {
    vkDestroyDescriptorPool(*m_deviceHandler, descriptorPool, nullptr);
}
} // namespace descriptor_set
