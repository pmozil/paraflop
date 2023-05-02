#pragma once
#include "common.hpp"

namespace device {

class DeviceHandler {
  public:
    DeviceHandler(std::vector<const char *> &deviceExtensions,
                  std::optional<std::vector<const char *>> &validationLayers,
                  VkInstance *instance, VkSurfaceKHR *surface)
        : deviceExtensions(deviceExtensions),
          validationLayers(validationLayers), instance(instance),
          surface(surface) {
        pickPhysicalDevice();
        createLogicalDevice(nullptr);
    };
    VkPhysicalDevice &getPhysicalDevice() { return physicalDevice; };
    VkDevice &getLogicalDevice() { return logicalDevice; };
    VkQueue &getGraphicsQueue() { return graphicsQueue; };
    VkQueue &getPresentQueue() { return presentQueue; };
    QueueFamilyIndices getQueueFamilyIndices();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    inline void cleanupDevice(VkAllocationCallbacks *pAllocator) {
        vkDestroyDevice(logicalDevice, pAllocator);
    }

  private:
    std::vector<const char *> &deviceExtensions;
    std::optional<std::vector<const char *>> &validationLayers;
    VkInstance *instance;
    VkSurfaceKHR *surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool deviceIsSuitable(VkPhysicalDevice device);
    void pickPhysicalDevice();
    void createLogicalDevice(VkAllocationCallbacks *pAllocator);
    int rateDevice(VkPhysicalDevice device);
};
} // namespace device
