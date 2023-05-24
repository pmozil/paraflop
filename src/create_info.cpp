#include "vulkan_utils/create_info.hpp"

namespace create_info {
VkBufferCreateInfo bufferCreateInfo(VkDeviceSize size,
                                    VkBufferUsageFlags usageFlags,
                                    VkSharingMode sharingMode) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usageFlags;
  bufferInfo.sharingMode = sharingMode;
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
VkImageViewCreateInfo imageViewCreateInfo(VkImage image, VkFormat format) {
  VkImageViewCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.image = image;
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format = format;
  createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.levelCount = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.layerCount = 1;
  return createInfo;
}

VkSwapchainCreateInfoKHR
swapChainCreateInfo(VkSurfaceKHR surface, uint32_t imageCount, VkFormat format,
                    VkColorSpaceKHR colorSpace, VkExtent2D extent,
                    uint32_t imageArrayLayers, VkImageUsageFlags usageFlags,
                    QueueFamilyIndices indices,
                    VkSurfaceTransformFlagBitsKHR currentTransform,
                    VkPresentModeKHR presentMode) {
  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = format;
  createInfo.imageColorSpace = colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = imageArrayLayers;
  createInfo.imageUsage = usageFlags;

  std::array<uint32_t, 2> queueFamilyIndices = {indices.graphicsFamily.value(),
                                                indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  return createInfo;
}

VkDescriptorSetLayoutBinding
descriptorSetLayoutBinding(VkDescriptorType descType,
                           VkShaderStageFlags stageFlags, uint32_t binding,
                           uint32_t descriptors) {
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = binding;
  uboLayoutBinding.descriptorCount = descriptors;
  uboLayoutBinding.descriptorType = descType;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = stageFlags;

  return uboLayoutBinding;
}

VkDescriptorSetLayoutCreateInfo
descriptorSetLayoutInfo(VkDescriptorSetLayoutBinding *bindings,
                        uint32_t bind_count) {
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = bind_count;
  layoutInfo.pBindings = bindings;

  return layoutInfo;
}

VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(VkDescriptorType type,
                                                    uint32_t poolSizeInt) {
  VkDescriptorPoolSize poolSize{};
  poolSize.type = type;
  poolSize.descriptorCount = poolSizeInt;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  return poolInfo;
}

VkDescriptorSetAllocateInfo
descriptorSetAllocInfo(VkDescriptorPool descriptorPool,
                       std::vector<VkDescriptorSetLayout> &layouts) {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = layouts.size();
  allocInfo.pSetLayouts = layouts.data();
  return allocInfo;
}
} // namespace create_info
