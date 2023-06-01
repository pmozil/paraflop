#pragma once
#include "common.hpp"

namespace surface {
/** \fn VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window,
                           VkAllocationCallbacks *allocator);
    \brief create a surface

    \param instance the VK instance
    \param window the GLFW window
    \param allocator the custom allocator

    \return The VK surface
*/
VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window,
                           VkAllocationCallbacks *allocator);

/** \fn void cleanupSurface(VkInstance instance, VkSurfaceKHR surface,
                    VkAllocationCallbacks *palloc);

    \brief Destroy the surface

    \param instance the VK instance
    \param surface the surface
    \param allocator the custom allocator
*/
void cleanupSurface(VkInstance instance, VkSurfaceKHR surface,
                    VkAllocationCallbacks *palloc);

/** \class Surface
 *
 * \brief The surface class
 *
 * Handles surface creation and destruction
 */
class Surface {
  public:
    VkInstance instance;  /**< The vk instance */
    VkSurfaceKHR surface; /**< The k surface */
    Surface(VkInstance instance, GLFWwindow *window,
            VkAllocationCallbacks *alloc)
        : instance(instance), surface(createSurface(instance, window, alloc)) {}
    ~Surface() { cleanupSurface(instance, surface, nullptr); }
};
} // namespace surface
