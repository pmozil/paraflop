#include "surface.hpp"

VkSurfaceKHR surface::createSurface(VkInstance &instance, GLFWwindow *window,
                                    VkAllocationCallbacks *allocator) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, allocator, &surface) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    return surface;
}

void surface::cleanupSurface(VkInstance &instance, VkSurfaceKHR &surface,
                             VkAllocationCallbacks *palloc) {
    vkDestroySurfaceKHR(instance, surface, palloc);
}
