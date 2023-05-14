#pragma once
#include "vulkan_utils/common.hpp"

namespace device {
const int DISCRETE_GPU_BONUS = 1000;
const int INTEGRATED_GPU_BONUS = 200;

class DeviceHandler {
  public:
    DeviceHandler(std::vector<const char *> &, std::vector<const char *> &,
                  VkInstance, VkSurfaceKHR);
    QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice &device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice &device);
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;
    inline VkQueue getTransferQueue() {
        return transferQueue != VK_NULL_HANDLE ? transferQueue : graphicsQueue;
    }
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    [[nodiscard]] VkCommandPool createCommandPool(
        uint32_t queueFamilyIndex,
        VkCommandPoolCreateFlags createFlags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const;
    [[nodiscard]] VkCommandBuffer
    createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool,
                        bool begin = false) const;
    inline void cleanupDevice(VkAllocationCallbacks *pAllocator) const {
        vkDestroyDevice(logicalDevice, pAllocator);
    }

  private:
    std::vector<const char *> &m_deviceExtensions;
    std::vector<const char *> &m_validationLayers;
    VkInstance m_vkInstance;
    VkSurfaceKHR m_vkSurface;
    bool m_checkDeviceExtensions(VkPhysicalDevice device);
    bool m_deviceIsSuitable(VkPhysicalDevice device);
    void m_pickDevice();
    void m_createLogicalDevice(VkAllocationCallbacks *pAllocator);
    int m_rateDevice(VkPhysicalDevice device);
};
} // namespace device
