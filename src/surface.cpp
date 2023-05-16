#include "vulkan_utils/surface.hpp"

namespace surface {
VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window,
                           VkAllocationCallbacks *allocator) {
  VkSurfaceKHR surface;
  VK_CHECK(glfwCreateWindowSurface(instance, window, allocator, &surface));
  return surface;
}

void cleanupSurface(VkInstance instance, VkSurfaceKHR surface,
                    VkAllocationCallbacks *palloc) {
  vkDestroySurfaceKHR(instance, surface, palloc);
}
} // namespace surface
