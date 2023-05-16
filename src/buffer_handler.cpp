#include "vulkan_utils/buffer_handler.hpp"
#include "vulkan_utils/create_info.hpp"

namespace descriptor_set {

template <typename Buffer>
BufferHandler<Buffer>::BufferHandler(
    std::shared_ptr<device::DeviceHandler> m_deviceHandler,
    std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer,
    VkDeviceSize m_bufferRange, VkDescriptorType m_descriptorType,
    VkShaderStageFlags m_stageFlags, uint32_t m_framesInFlight)
    : m_deviceHandler(std::move(m_deviceHandler)),
      m_commandBuffer(std::move(m_commandBuffer)), m_bufferRange(m_bufferRange),
      m_descriptorType(m_descriptorType), m_stageFlags(m_stageFlags),
      m_framesInFlight(m_framesInFlight) {
    createDescriptorSetLayout();
    createBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

template <typename Buffer>
void BufferHandler<Buffer>::createDescriptorSetLayout() {
    VkDescriptorSetLayoutCreateInfo layoutInfo =
        create_info::descriptorSetLayoutInfo(m_descriptorType, m_stageFlags);

    VK_CHECK(vkCreateDescriptorSetLayout(m_deviceHandler->logicalDevice,
                                         &layoutInfo, nullptr,
                                         &descriptorSetLayout));
}

template <typename Buffer> void BufferHandler<Buffer>::createBuffers() {
    buffers.resize(m_framesInFlight);

    for (int i = 0; i < m_framesInFlight; i++) {
        buffers[i] = Buffer(m_deviceHandler, m_commandBuffer);
    }
}

template <typename Buffer> void BufferHandler<Buffer>::createDescriptorPool() {
    VkDescriptorPoolCreateInfo poolInfo = create_info::descriptorPoolCreateInfo(
        m_descriptorType, m_framesInFlight);

    VK_CHECK(vkCreateDescriptorPool(m_deviceHandler->logicalDevice, &poolInfo,
                                    nullptr, &descriptorPool));
}

template <typename Buffer> void BufferHandler<Buffer>::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight,
                                               descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = m_framesInFlight;
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(m_framesInFlight);
    VK_CHECK(vkAllocateDescriptorSets(m_deviceHandler->logicalDevice,
                                      &allocInfo, descriptorSets.data()));

    for (size_t i = 0; i < m_framesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = m_bufferRange;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = m_descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_deviceHandler->logicalDevice, 1,
                               &descriptorWrite, 0, nullptr);
    }
}
} // namespace descriptor_set
