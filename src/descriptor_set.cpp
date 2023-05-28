#include "vulkan_utils/descriptor_set.hpp"
#include "geometry/uniform_buffer_object.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/device.hpp"

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

    VK_CHECK(vkCreateDescriptorSetLayout(m_deviceHandler->logicalDevice,
                                         &createInfo, nullptr, &layout));
}

void DescriptorSetLayout::cleanup() {
    vkDestroyDescriptorSetLayout(m_deviceHandler->logicalDevice, layout,
                                 nullptr);
}

DescriptorSetHandler::DescriptorSetHandler(
    std::shared_ptr<device::DeviceHandler> m_deviceHandler,
    std::shared_ptr<DescriptorSetLayout> m_layout,
    std::shared_ptr<buffer::Buffer> buffer)
    : m_deviceHandler(std::move(m_deviceHandler)),
      m_layout(std::move(m_layout)), m_buffer(std::move(buffer)) {
    m_createDescriptorPool();
    m_createDescriptorSets();
}

void DescriptorSetHandler::m_createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VK_CHECK(vkCreateDescriptorPool(m_deviceHandler->logicalDevice, &poolInfo,
                                    nullptr, &descriptorPool));
}

void DescriptorSetHandler::m_createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                               m_layout->layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts.data();

    VK_CHECK(vkAllocateDescriptorSets(m_deviceHandler->logicalDevice,
                                      &allocInfo, &descriptorSet));

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_buffer->buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(m_deviceHandler->logicalDevice, 1, &descriptorWrite,
                           0, nullptr);
}

void DescriptorSetHandler::cleanup() {
    vkDestroyDescriptorPool(m_deviceHandler->logicalDevice, descriptorPool,
                            nullptr);
}
} // namespace descriptor_set
