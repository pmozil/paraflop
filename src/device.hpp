#pragma once
#include "common.hpp"

namespace device {

class DeviceHandler {
  public:
    VkPhysicalDevice &getPhysicalDevice() { return physicalDevice; };
    VkDevice &getLogicalDevice() { return logicalDevice; };
    QueueFamilyIndices getQueueFamilyIndices();
    DeviceHandler(VkInstance &inst, VkSurfaceKHR &surf)
        : instance(inst), surface(surf) {
        pickPhysicalDevice();
        createLogicalDevice(nullptr);
    };

  private:
    VkInstance &instance;
    VkSurfaceKHR &surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool deviceIsSuitable(VkPhysicalDevice device);
    void pickPhysicalDevice();
    void createLogicalDevice(VkAllocationCallbacks *pAllocator);
    int rateDevice(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
};
} // namespace device
