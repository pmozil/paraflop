#pragma once
#include "common.hpp"

namespace device {
const int DISCRETE_GPU_BONUS = 1000;
const int INTEGRATED_GPU_BONUS = 200;

class DeviceHandler {
  public:
    DeviceHandler(std::vector<const char *> &, std::vector<const char *> &,
                  VkInstance &, VkSurfaceKHR &);
    VkPhysicalDevice getPhysicalDevice() { return physicalDevice; };
    VkDevice getLogicalDevice() { return logicalDevice; };
    VkQueue getGraphicsQueue() { return graphicsQueue; };
    VkQueue getPresentQueue() { return presentQueue; };
    QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    inline void cleanupDevice(VkAllocationCallbacks *pAllocator) {
        vkDestroyDevice(logicalDevice, pAllocator);
    }

  private:
    std::vector<const char *> &deviceExtensions;
    std::vector<const char *> &validationLayers;
    VkInstance &instance;
    VkSurfaceKHR &surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool deviceIsSuitable(VkPhysicalDevice device);
    void pickPhysicalDevice();
    void createLogicalDevice(VkAllocationCallbacks *pAllocator);
    int rateDevice(VkPhysicalDevice device);
};
} // namespace device
