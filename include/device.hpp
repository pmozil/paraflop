#pragma once
#include "common.hpp"

namespace device {
const int DISCRETE_GPU_BONUS = 1000;
const int INTEGRATED_GPU_BONUS = 200;

class DeviceHandler {
  public:
    DeviceHandler(std::vector<const char *> &, std::vector<const char *> &,
                  VkInstance &, VkSurfaceKHR &);
    QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice &device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice &device);
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkCommandPool
    createCommandPool(uint32_t queueFamilyIndex,
                      VkCommandPoolCreateFlags createFlags =
                          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level,
                                        VkCommandPool pool, bool begin = false);
    inline void cleanupDevice(VkAllocationCallbacks *pAllocator) {
        vkDestroyDevice(logicalDevice, pAllocator);
    }

  private:
    std::vector<const char *> &deviceExtensions;
    std::vector<const char *> &validationLayers;
    VkInstance &instance;
    VkSurfaceKHR &surface;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool deviceIsSuitable(VkPhysicalDevice device);
    void pickPhysicalDevice();
    void createLogicalDevice(VkAllocationCallbacks *pAllocator);
    int rateDevice(VkPhysicalDevice device);
};
} // namespace device
