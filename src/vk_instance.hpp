#pragma once
#include "common.hpp"
#include "debug.hpp"

namespace vk_instance {

bool checkValidationLayerSupport(VkLayerProperties *pProperties);

VkInstance createDefaultVkInstance(VkAllocationCallbacks *alloc);

VkInstance createNamedVkInstance(char *name, VkAllocationCallbacks *alloc);

std::vector<const char *> getRequiredExtensions();

void cleanupInstance(VkInstance &instance, VkAllocationCallbacks *palloc);
} // namespace vk_instance
