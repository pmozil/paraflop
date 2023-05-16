#pragma once
#include "common.hpp"
#include "vulkan_utils/debug.hpp"

namespace vk_instance {

bool checkValidationLayerSupport(VkLayerProperties *pProperties);

VkInstance createDefaultVkInstance(VkAllocationCallbacks *alloc);

VkInstance createNamedVkInstance(char *name, VkAllocationCallbacks *alloc);

std::vector<const char *> getRequiredExtensions();

inline void cleanupInstance(VkInstance &instance,
                            VkAllocationCallbacks *palloc) {
  vkDestroyInstance(instance, palloc);
}

class Instance {
public:
  Instance(char *name) : instance(createNamedVkInstance(name, nullptr)) {}
  Instance() : instance(createDefaultVkInstance(nullptr)) {}
  ~Instance() { cleanupInstance(instance, nullptr); }
  VkInstance instance;
};
} // namespace vk_instance
