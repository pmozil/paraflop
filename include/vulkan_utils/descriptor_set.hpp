#pragma once

#include "common.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace descriptor_set {
class DescriptorSetLayout {
  public:
    DescriptorSetLayout(std::shared_ptr<device::DeviceHandler> m_deviceHandler,
                        VkDescriptorType type, VkShaderStageFlags flags);
    VkDescriptorSetLayout layout;

  private:
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    void createLayout(VkDescriptorType type, VkShaderStageFlags flags);
};

class DescriptorSetHandler {};
} // namespace descriptor_set
