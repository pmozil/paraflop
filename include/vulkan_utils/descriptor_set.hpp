#pragma once

#include "common.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace descriptor_set {
class DescriptorSetLayout {
  public:
    DescriptorSetLayout(std::shared_ptr<device::DeviceHandler> m_deviceHandler,
                        VkDescriptorSetLayoutBinding *bindings,
                        uint32_t bind_count);
    ~DescriptorSetLayout() { cleanup(); }
    VkDescriptorSetLayout layout;
    void cleanup();

  private:
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    void m_createLayout(VkDescriptorSetLayoutBinding *bindings,
                        uint32_t bind_count);
};

class DescriptorSetHandler {
  public:
    DescriptorSetHandler(std::shared_ptr<device::DeviceHandler> m_deviceHandler,
                         std::shared_ptr<DescriptorSetLayout> m_layout,
                         std::vector<buffer::Buffer> &buffers);
    ~DescriptorSetHandler() { cleanup(); }
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    void cleanup();

  private:
    void m_createDescriptorSets();
    void m_createDescriptorPool();
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    std::shared_ptr<DescriptorSetLayout> m_layout;
    std::vector<buffer::Buffer> &m_buffers;
};
} // namespace descriptor_set
