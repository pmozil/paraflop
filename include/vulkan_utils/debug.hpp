#pragma once
#include "common.hpp"

namespace debug {

/** \fn VkResult CreateDebugUtilsMessengerEXT(
    VkInstance &instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger);

    \brief Create a debug messenger for vulkan instance

    \param instance the VK instance
    \param pCreateInfo debug create info
    \param pAllocator a custom allocator
    \param pDebugMessenger the debug messenger

    \return The result for creating the mesenger
*/
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance &instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger);

/** \fn void DestroyDebugUtilsMessengerEXT(VkInstance &instance,
                                   VkDebugUtilsMessengerEXT &debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);
    \brief Destroy the debug messenger

    \param instance the VK instance
    \param pDebugMessenger the messenger
    \param pAllocator the custom allocator
*/
void DestroyDebugUtilsMessengerEXT(VkInstance &instance,
                                   VkDebugUtilsMessengerEXT &debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);

/** \fn void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    \brief populate createInfo with required data
*/
void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo);

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance &instance);

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
    UNUSED(messageSeverity);
    UNUSED(messageType);
    UNUSED(pCallbackData);
    UNUSED(pUserData);
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
} /**< the debug callback */
} // namespace debug
