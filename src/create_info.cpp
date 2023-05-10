#include "create_info.hpp"

namespace create_info {
VkBufferCreateInfo bufferCreateInfo(VkDeviceSize size,
                                    VkBufferUsageFlags usageFlags) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return bufferInfo;
}

VkMemoryAllocateInfo memoryAllocInfo(VkDeviceSize size, uint32_t memoryIndex) {
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = memoryIndex;
    return allocInfo;
}

VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    return poolInfo;
}

VkCommandBufferAllocateInfo
commandBuffferAllocInfo(VkCommandPool commandPool,
                        uint32_t commandBufferCount) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBufferCount;
    return allocInfo;
}

VkCommandBufferBeginInfo commabdBufferBeginInfo() {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    return beginInfo;
}

VkSubmitInfo submitInfo(uint32_t commandBufferCount, VkCommandBuffer *buffers) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = buffers;
    return submitInfo;
}

VkBufferCopy copyRegion(VkDeviceSize size) {
    VkBufferCopy region{};
    region.size = size;
    return region;
}

VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass,
                                          VkFramebuffer framebuffer,
                                          VkExtent2D extent,
                                          const VkClearValue *clearColor) {

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = clearColor;
    return renderPassInfo;
}

VkDeviceQueueCreateInfo queueCreateInfo(uint32_t queueFamily,
                                        const float *queuePriority) {

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriority;
    return queueCreateInfo;
}

VkPipelineShaderStageCreateInfo
shaderStageInfo(VkShaderModule shaderModule,
                VkShaderStageFlagBits shaderStage) {
    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = shaderStage;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = "main";
    return shaderStageInfo;
}

VkDeviceCreateInfo
deviceCreateInfo(std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos,
                 std::vector<const char *> &deviceExtensions,
                 std::vector<const char *> &validationLayers,
                 const VkPhysicalDeviceFeatures *deviceFeatures) {
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = deviceFeatures;

    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (!validationLayers.empty()) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    return createInfo;
}
} // namespace create_info
