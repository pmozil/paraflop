#include "common.hpp"

namespace surface {
VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window,
                           VkAllocationCallbacks *allocator);

void cleanupSurface(VkInstance instance, VkSurfaceKHR surface,
                    VkAllocationCallbacks *palloc);
} // namespace surface
