#include "device.hpp"
#include "create_info.hpp"

namespace device {
bool DeviceHandler::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    for (const auto &extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails
DeviceHandler::querySwapChainSupport(VkPhysicalDevice &device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                              &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                             details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool DeviceHandler::deviceIsSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = getQueueFamilyIndices(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport =
            querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() &&
                            !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

int DeviceHandler::rateDevice(VkPhysicalDevice device) {
    if (!deviceIsSuitable(device)) {
        return -1;
    }

    int score = 0;

    VkPhysicalDeviceProperties deviceProps;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProps);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    score += deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
                 ? DISCRETE_GPU_BONUS
                 : INTEGRATED_GPU_BONUS;
    score += deviceProps.limits.maxImageDimension2D;
    score *= deviceFeatures.geometryShader;

    return score;
}

void DeviceHandler::pickPhysicalDevice() {
    std::multimap<int, VkPhysicalDevice> candidates;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &device : devices) {
        int score = rateDevice(device);
        candidates.insert(std::make_pair(score, device));
    }
    if (candidates.rbegin()->first > 0) {
        physicalDevice = candidates.rbegin()->second;
    } else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void DeviceHandler::createLogicalDevice(VkAllocationCallbacks *pAllocator) {
    QueueFamilyIndices indices = getQueueFamilyIndices(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentFamily.value()};

    float queuePriority = 1.0F;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo =
            create_info::queueCreateInfo(queueFamily, &queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = create_info::deviceCreateInfo(
        queueCreateInfos, deviceExtensions, validationLayers, &deviceFeatures);

    VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, pAllocator,
                            &logicalDevice));

    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0,
                     &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0,
                     &presentQueue);
}

QueueFamilyIndices
DeviceHandler::getQueueFamilyIndices(VkPhysicalDevice &device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    int idx = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = idx;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, surface,
                                             &presentSupport);

        if (presentSupport) {
            indices.presentFamily = idx;
        }

        if (indices.isComplete()) {
            break;
        }

        idx++;
    }

    return indices;
}

DeviceHandler::DeviceHandler(std::vector<const char *> &devExt,
                             std::vector<const char *> &validations,
                             VkInstance &instance, VkSurfaceKHR &surface)
    : deviceExtensions(devExt), validationLayers(validations),
      instance(instance), surface(surface) {
    pickPhysicalDevice();
    createLogicalDevice(nullptr);
}

VkCommandPool
DeviceHandler::createCommandPool(uint32_t queueFamilyIndex,
                                 VkCommandPoolCreateFlags createFlags) {
    VkCommandPoolCreateInfo commandPoolCreateInfo =
        create_info::commandPoolCreateInfo(queueFamilyIndex);
    commandPoolCreateInfo.flags = createFlags;
    VkCommandPool commandPool;
    VK_CHECK(vkCreateCommandPool(logicalDevice, &commandPoolCreateInfo, nullptr,
                                 &commandPool));
    return commandPool;
}

VkCommandBuffer DeviceHandler::createCommandBuffer(VkCommandBufferLevel level,
                                                   VkCommandPool pool,
                                                   bool begin) {
    VkCommandBufferAllocateInfo allocInfo =
        create_info::commandBuffferAllocInfo(pool, 1);
    allocInfo.level = level;

    VkCommandBuffer cmdBuffer;
    VK_CHECK(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &cmdBuffer));
    // If requested, also start recording for the new command buffer
    if (begin) {
        VkCommandBufferBeginInfo bufferBeginInfo{};
        bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &bufferBeginInfo));
    }
    return cmdBuffer;
}
} // namespace device
