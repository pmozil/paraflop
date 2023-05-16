#pragma once
#include "common.hpp"

namespace surface {
VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window,
                           VkAllocationCallbacks *allocator);

void cleanupSurface(VkInstance instance, VkSurfaceKHR surface,
                    VkAllocationCallbacks *palloc);

class Surface {
public:
  VkInstance instance;
  VkSurfaceKHR surface;
  Surface(VkInstance instance, GLFWwindow *window, VkAllocationCallbacks *alloc)
      : instance(instance), surface(createSurface(instance, window, alloc)) {}
  ~Surface() { cleanupSurface(instance, surface, nullptr); }
};
} // namespace surface
