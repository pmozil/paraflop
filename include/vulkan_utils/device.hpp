#pragma once
#include "common.hpp"

namespace device {
const int DISCRETE_GPU_BONUS = 1000;  /**< Bonus for descrete GPU */
const int INTEGRATED_GPU_BONUS = 200; /**< Bonus for integrated GPU */

/**
 * \class DeviceHandler
 *
 * \brief Handler for Vulkan device operations.
 *
 * The DeviceHandler class provides functionality for managing Vulkan devices,
 * such as querying queue family indices, swap chain support details, creating
 * command pools and buffers, and cleaning up the device.
 */
class DeviceHandler {
  public:
    /**
     * \fn DeviceHandler(std::vector<const char *> &, std::vector<const char *>
     * &, VkInstance, VkSurfaceKHR)
     *
     * \brief Constructs a DeviceHandler object.
     *
     * \param deviceExtensions The list of device extensions required by the
     * application.
     * \param validationLayers The list of validation layers
     * enabled for debugging.
     * \param vkInstance The Vulkan instance associated
     * with the device.
     * \param vkSurface The Vulkan surface associated with the
     * device.
     */
    DeviceHandler(std::vector<const char *> &deviceExtensions,
                  std::vector<const char *> &validationLayers,
                  VkInstance vkInstance, VkSurfaceKHR vkSurface);

    /**
     * \fn ~DeviceHandler()
     *
     * \brief Destructor for the DeviceHandler class.
     */
    ~DeviceHandler() { cleanupDevice(nullptr); }

    /**
     * \fn VkDevice()
     *
     * \brief Operator for DeviceHandler to be used like VkDevice
     *
     * \return A VkDevice
     * */
    operator VkDevice() const { return logicalDevice; };

    /**
     * \fn QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice &device)
     *
     * \brief Retrieves the queue family indices for the specified physical
     * device.
     *
     * \param device The physical device to query for queue family indices.
     *
     * \return The QueueFamilyIndices object containing the queue family
     * indices.
     */
    QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice &device);

    /**
     * \fn SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice
     * &device)
     *
     * \brief Queries the swap chain support details for the specified
     * physical device.
     *
     * \param device The physical device to query for swap chain support
     * details.
     *
     * \return The SwapChainSupportDetails object containing the swap
     * chain support details.
     */
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice &device);

    VkQueue graphicsQueue = VK_NULL_HANDLE; /**< The graphics queue. */
    VkQueue presentQueue = VK_NULL_HANDLE;  /**< The present queue. */
    VkQueue transferQueue = VK_NULL_HANDLE; /**< The transfer queue. */

    /**
     * \fn inline VkQueue getTransferQueue()
     *
     * \brief Retrieves the transfer queue if available, otherwise returns the
     * graphics queue.
     *
     * \return The transfer queue if available, otherwise the graphics queue.
     */
    inline VkQueue getTransferQueue() {
        return transferQueue != VK_NULL_HANDLE ? transferQueue : graphicsQueue;
    }

    VkPhysicalDevice physicalDevice =
        VK_NULL_HANDLE;                      /**< The physical device. */
    VkDevice logicalDevice = VK_NULL_HANDLE; /**< The logical device. */

    /**
     * \fn VkCommandPool createCommandPool(uint32_t
     * queueFamilyIndex, VkCommandPoolCreateFlags createFlags =
     * VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
     *
     * \brief Creates a
     * command pool for the specified queue family index.
     *
     * \param queueFamilyIndex The index of the queue family associated with the
     * command pool.
     * \param createFlags The optional create flags for the
     * command pool.
     *
     * \return The created command pool.
     */
    [[nodiscard]] VkCommandPool createCommandPool(
        uint32_t queueFamilyIndex,
        VkCommandPoolCreateFlags createFlags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const;

    /**
     * \fn VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level,
     * VkCommandPool pool, bool begin = false)
     *
     * \brief Creates a command
     * buffer of the specified level from the given command pool.
     *
     * \param level The level of the command buffer.
     * \param pool The command pool from which to allocate the command buffer.
     * \param begin Whether to begin the command buffer recording.
     *
     * \return The created command buffer.
     */
    [[nodiscard]] VkCommandBuffer
    createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool,
                        bool begin = false) const;

    /**
     * \fn inline void cleanupDevice(VkAllocationCallbacks *pAllocator) const
     *
     * \brief Cleans up the logical device.
     *
     * \param pAllocator The optional allocator to use for device cleanup.
     */
    inline void cleanupDevice(VkAllocationCallbacks *pAllocator) const {
        vkDestroyDevice(logicalDevice, pAllocator);
    }

  private:
    std::vector<const char *>
        &m_deviceExtensions; /**< The required device extensions. */
    std::vector<const char *>
        &m_validationLayers; /**< The enabled validation layers. */
    VkInstance
        m_vkInstance; /**< The Vulkan instance associated with the device. */
    VkSurfaceKHR
        m_vkSurface; /**< The Vulkan surface associated with the device. */

    /**
     * \fn bool m_checkDeviceExtensions(VkPhysicalDevice device)
     *
     * \brief Checks if the specified physical device supports all the required
     * device extensions.
     *
     * \param device The physical device to check.
     *
     * \return True if all device extensions are supported, false otherwise.
     */
    bool m_checkDeviceExtensions(VkPhysicalDevice device);

    /**
     * \fn bool m_deviceIsSuitable(VkPhysicalDevice device)
     *
     * \brief Checks if the specified physical device is suitable for use.
     *
     * \param device The physical device to check.
     *
     * \return True if the device is suitable, false otherwise.
     */
    bool m_deviceIsSuitable(VkPhysicalDevice device);

    /**
     * \fn void m_pickDevice()
     * \brief Picks the most suitable physical device based on its rating.
     */
    void m_pickDevice();

    /**
     * \fn void m_createLogicalDevice(VkAllocationCallbacks *pAllocator)
     *
     * \brief Creates the logical device.
     *
     * \param pAllocator The optional allocator to use for device creation.
     */
    void m_createLogicalDevice(VkAllocationCallbacks *pAllocator);

    /**
     * \fn int m_rateDevice(VkPhysicalDevice device)
     *
     * \brief Rates the suitability of the specified physical device.
     *
     * \param device The physical device to rate.
     *
     * \return The rating of the device.
     */
    int m_rateDevice(VkPhysicalDevice device);
};
} // namespace device
