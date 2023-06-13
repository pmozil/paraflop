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
    DeviceHandler(std::vector<const char *> devExt,
                  std::vector<const char *> validations, VkInstance vkInstance,
                  VkSurfaceKHR vkSurface);

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
    VkPhysicalDeviceMemoryProperties
        memoryProperties; /**< The device memory properties */
    VkPhysicalDeviceFeatures
        enabledFeatures; /**< Features that have been enabled for use on the
                            physical device */
    VkPhysicalDeviceProperties
        properties; /**< The device physica properties, e. g. memory */

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
     * \fn inline void cleanupDevice(VkAllocationCallbacks *pAllocator) const
     *
     * \brief Cleans up the logical device.
     *
     * \param pAllocator The optional allocator to use for device cleanup.
     */
    inline void cleanupDevice(VkAllocationCallbacks *pAllocator) const {
        vkDestroyDevice(logicalDevice, pAllocator);
    }
    /**
     * Get the index of a memory type that has all the requested property bits
     * set
     * \fn uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags
     *          properties, VkBool32 *memTypeFound) const;
     *
     * \param typeBits Bit mask with bits set for each memory type supported by
     * the resource to request for (from VkMemoryRequirements) \param properties
     * Bit mask of properties for the memory type to request \param (Optional)
     * memTypeFound Pointer to a bool that is set to true if a matching memory
     * type has been found
     *
     * \return Index of the requested memory type
     *
     * \throw Throws an exception if memTypeFound is null and no memory type
     * could be found that supports the requested properties
     */
    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties,
                           VkBool32 *memTypeFound = nullptr) const;

    /**
     * Create a buffer on the device
     *
     * \param usageFlags Usage flag bit mask for the buffer (i.e. index, vertex,
     * uniform buffer)
     * \param memoryPropertyFlags Memory properties for this
     * buffer (i.e. device local, host visible, coherent)
     * \param size Size of
     * the buffer in byes
     * \param buffer Pointer to the buffer handle acquired by
     * the function
     * \param memory Pointer to the memory handle acquired by the
     * function
     * \param data Pointer to the data that should be copied to the
     * buffer after creation (optional, if not set, no data is copied over)
     *
     * \return VK_SUCCESS if buffer handle and memory have been created and
     * (optionally passed) data has been copied
     */
    VkResult createBuffer(VkBufferUsageFlags usageFlags,
                          VkMemoryPropertyFlags memoryPropertyFlags,
                          VkDeviceSize size, VkBuffer *buffer,
                          VkDeviceMemory *memory, void *data) const;

  private:
    std::vector<const char *>
        m_deviceExtensions; /**< The required device extensions. */
    std::vector<const char *>
        m_validationLayers;  /**< The enabled validation layers. */
    VkInstance m_vkInstance; /**< The Vulkan instance associated with the
                                device. */
    VkSurfaceKHR
        m_vkSurface; /**< The Vulkan surface associated with the device. */

    /**
     * \fn bool m_checkDeviceExtensions(VkPhysicalDevice device)
     *
     * \brief Checks if the specified physical device supports all the
     * required device extensions.
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
