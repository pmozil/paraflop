#pragma once

#include "common.hpp"
#include "geometry/uniform_buffer_objects.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/create_info.hpp"
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
                         std::shared_ptr<buffer::Buffer> buffer);
    ~DescriptorSetHandler() { cleanup(); }
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    void cleanup();

  private:
    void m_createDescriptorSets();
    void m_createDescriptorPool();
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    std::shared_ptr<DescriptorSetLayout> m_layout;
    std::shared_ptr<buffer::Buffer> m_buffer;
};
} // namespace descriptor_set
