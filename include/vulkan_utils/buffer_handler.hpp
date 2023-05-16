#pragma once
#include "common.hpp"
#include "geometry/uniform_buffer_object.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace descriptor_set {
template <typename Buffer> class BufferHandler {
    static_assert(std::is_base_of<buffer::Buffer, Buffer>::value,
                  "The buffer object must inherit from buffer::Buffer");

  public:
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<buffer::Buffer> buffers;

    BufferHandler(
        std::shared_ptr<device::DeviceHandler> m_deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer,
        VkDeviceSize m_bufferRange, VkDescriptorType m_descriptorType,
        VkShaderStageFlags m_stageFlags, uint32_t m_framesInFlight);

    void cleanup();

  private:
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer;
    VkDeviceSize m_bufferRange;
    VkDescriptorType m_descriptorType;
    VkShaderStageFlags m_stageFlags;
    uint32_t m_framesInFlight;

    void createBuffers();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
};
} // namespace descriptor_set
