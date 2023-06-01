#pragma once
#include "common.hpp"
#include "vulkan_utils/debug.hpp"

namespace vk_instance {

/** \fn checkValidationLayerSupport(VkLayerProperties *pProperties)
 * \brief Check if device supports velidation layers
 *
 * \param pProperties the validation layer properties
 *
 * \return  whether the device supports given validation layers
 */
bool checkValidationLayerSupport(VkLayerProperties *pProperties);

/** \fn createDefaultVkInstance(VkAllocationCallbacks *alloc)
 *
 * \brief Create a vlukan instance
 *
 * \param alloc A custom allocator callback
 * \return  The vk instnce
 */
VkInstance createDefaultVkInstance(VkAllocationCallbacks *alloc);

/** \fn createDefaultVkInstance(VkAllocationCallbacks *alloc)
 *
 * \brief Create a vlukan instance
 *
 * \param name The app name
 * \param alloc A custom allocator callback
 * \return  The vk instnce
 */
VkInstance createNamedVkInstance(char *name, VkAllocationCallbacks *alloc);

/** \fn getRequiredExtensions()
 *
 * \brief get required extensions for the application
 *
 * \return
 */
std::vector<const char *> getRequiredExtensions();

/** \fn cleanupInstance(VkInstance &instance, VkAllocationCallbacks *palloc)
 *
 * \brief Destroy a VK instance
 */
inline void cleanupInstance(VkInstance &instance,
                            VkAllocationCallbacks *palloc) {
    vkDestroyInstance(instance, palloc);
}

/** \class Instance
 *
 * \brief The class that handles instance creation
 */
class Instance {
  public:
    /** \fn Instance(char *name)
     * \brief Constructor for Instance
     */
    Instance(char *name) : instance(createNamedVkInstance(name, nullptr)) {}
    /** \fn Instance()
     * \brief The default for Instance
     */
    Instance() : instance(createDefaultVkInstance(nullptr)) {}
    ~Instance() { cleanupInstance(instance, nullptr); }
    VkInstance instance; /**< The vk instance */
};
} // namespace vk_instance
