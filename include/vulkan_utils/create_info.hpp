#pragma once
#include "common.hpp"

namespace create_info {
/**
 * \fn  VkBufferCreateInfo bufferCreateInfo(VkDeviceSize size,
 VkBufferUsageFlags usageFlags, VkSharingMode sharingMode =
 VK_SHARING_MODE_EXCLUSIVE);
 *
 * \brief Create a Vulkan buffer create info structure.
 *
 * This function creates a VkBufferCreateInfo structure with the specified
 * parameters.
 *
 * \param size The size of the buffer in bytes.
 * \param usageFlags The usage flags specifying how the buffer will be used.
 * \param sharingMode The sharing mode of the buffer (optional, default:
 * VK_SHARING_MODE_EXCLUSIVE).
 *
 * \return The created VkBufferCreateInfo structure.
 */
VkBufferCreateInfo
bufferCreateInfo(VkDeviceSize size, VkBufferUsageFlags usageFlags,
                 VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

/**
 * \fn VkMemoryAllocateInfo memoryAllocInfo(VkDeviceSize size, uint32_t
 * memoryIndex)
 *
 * \brief Create a Vulkan memory allocation info structure.
 *
 * This function creates a VkMemoryAllocateInfo structure with the specified
 * parameters.
 *
 * \param size The size of the memory allocation in bytes.
 * \param memoryIndex The index of the memory type to allocate from.
 *
 * \return The created VkMemoryAllocateInfo structure.
 */
VkMemoryAllocateInfo memoryAllocInfo(VkDeviceSize size, uint32_t memoryIndex);

/**
 * \fn VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex)
 *
 * \brief Create a Vulkan command pool create info structure.
 *
 * This function creates a VkCommandPoolCreateInfo structure with the specified
 * parameters.
 *
 * \param queueFamilyIndex The index of the queue family that commands will be
 * submitted to.
 *
 * \return The created VkCommandPoolCreateInfo structure.
 */
VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex);

/**
 * \fn VkCommandBufferAllocateInfo commandBufferAllocInfo(VkCommandPool
 * commandPool, uint32_t commandBufferCount)
 *
 * \brief Create a Vulkan command
 * buffer allocate info structure.
 *
 * This function creates a VkCommandBufferAllocateInfo structure with the
 * specified parameters.
 *
 * \param commandPool The command pool from which the command buffers will be
 * allocated. \param commandBufferCount The number of command buffers to
 * allocate.
 *
 * \return The created VkCommandBufferAllocateInfo structure.
 */
VkCommandBufferAllocateInfo commandBufferAllocInfo(VkCommandPool commandPool,
                                                   uint32_t commandBufferCount);

/**
 * \fn VkCommandBufferBeginInfo commandBufferBeginInfo()
 *
 * \brief Create a Vulkan command buffer begin info structure.
 *
 * This function creates a VkCommandBufferBeginInfo structure with default
 * values.
 *
 * \return The created VkCommandBufferBeginInfo structure.
 */
VkCommandBufferBeginInfo commandBufferBeginInfo();

/**
 * \fn VkSubmitInfo submitInfo(uint32_t commandBufferCount, VkCommandBuffer
 * *buffers)
 *
 * \brief Create a Vulkan submit info structure.
 *
 * \brief This function creates a VkSubmitInfo structure with the specified
 * parameters.
 *
 * \param commandBufferCount The number of command buffers to submit.
 * \param buffers An array of command buffers to submit.
 *
 * \return The created VkSubmitInfo structure.
 */
VkSubmitInfo submitInfo(uint32_t commandBufferCount, VkCommandBuffer *buffers);

/**
 * \fn VkBufferCopy copyRegion(VkDeviceSize size)
 *
 * \brief Create a Vulkan buffer copy region structure.
 *
 * \brief This function creates a VkBufferCopy structure with the specified
 * size.
 *
 * \param size The size of the buffer region to copy.
 *
 * \return The created VkBufferCopy structure.
 */
VkBufferCopy copyRegion(VkDeviceSize size);

/**
 * \fn VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass,
 * VkFramebuffer framebuffer, VkExtent2D extent, const VkClearValue *clearColor)
 *
 * \brief Create a Vulkan render pass begin info structure.
 *
 * \brief This function creates a VkRenderPassBeginInfo structure with the
 * specified parameters.
 *
 * \param renderPass The Vulkan render pass to begin.
 * \param framebuffer The Vulkan framebuffer to render into.
 * \param extent The size of the render area.
 * \param clearColor The clear values for each attachment in the render pass
 * (optional).
 *
 * \return The created VkRenderPassBeginInfo structure.
 */
VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass,
                                          VkFramebuffer framebuffer,
                                          VkExtent2D extent,
                                          const VkClearValue *clearColor);

/**
 * \fn VkDeviceQueueCreateInfo queueCreateInfo(uint32_t queueFamily, const float
 * *queuePriority)
 *
 * \brief Create a Vulkan device queue create info structure.
 *
 * \brief This function creates a VkDeviceQueueCreateInfo structure with the
 * specified parameters.
 *
 * \param queueFamily The index of the queue family.
 * \param queuePriority The priority of the queue.
 *
 * \return The created VkDeviceQueueCreateInfo structure.
 */
VkDeviceQueueCreateInfo queueCreateInfo(uint32_t queueFamily,
                                        const float *queuePriority);

/**
 * \fn VkPipelineShaderStageCreateInfo shaderStageInfo(VkShaderModule
 * shaderModule, VkShaderStageFlagBits shaderStage)
 *
 * \brief Create a Vulkan
 * pipeline shader stage create info structure.
 *
 * \brief This function creates a VkPipelineShaderStageCreateInfo structure with
 * the specified parameters.
 *
 * \param shaderModule The Vulkan shader module containing the shader code.
 * \param shaderStage The stage of the shader (e.g., vertex shader, fragment
 * shader, etc.).
 *
 * \return The created VkPipelineShaderStageCreateInfo structure.
 */
VkPipelineShaderStageCreateInfo
shaderStageInfo(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStage);

/**
 * \fn VkDeviceCreateInfo deviceCreateInfo(std::vector<VkDeviceQueueCreateInfo>&
 * queueCreateInfos, std::vector<const char*>& deviceExtensions,
 *                                          std::vector<const char*>&
 * validationLayers, const VkPhysicalDeviceFeatures* deviceFeatures) \brief
 * Creates a Vulkan device create info structure.
 *
 * \brief This function creates a VkDeviceCreateInfo structure with the
 * specified parameters.
 *
 * \param queueCreateInfos The vector of VkDeviceQueueCreateInfo structures for
 * specifying device queue creation. \param deviceExtensions The vector of
 * device extension names required by the device. \param validationLayers The
 * vector of validation layer names to enable for the device (optional). \param
 * deviceFeatures Pointer to the VkPhysicalDeviceFeatures structure specifying
 * the device features (optional).
 *
 * \return The created VkDeviceCreateInfo structure.
 */
VkDeviceCreateInfo
deviceCreateInfo(std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos,
                 std::vector<const char *> &deviceExtensions,
                 std::vector<const char *> &validationLayers,
                 const VkPhysicalDeviceFeatures *deviceFeatures);

/**
 * \fn VkImageViewCreateInfo imageViewCreateInfo(VkImage image, VkFormat format)
 * \brief Creates a Vulkan image view create info structure.
 *
 * \brief This function creates a VkImageViewCreateInfo structure with the
 * specified parameters.
 *
 * \param image The Vulkan image to create a view for.
 * \param format The format of the image view.
 *
 * \return The created VkImageViewCreateInfo structure.
 */
VkImageViewCreateInfo imageViewCreateInfo(VkImage image, VkFormat format);

/**
 * \fn VkSwapchainCreateInfoKHR swapChainCreateInfo(VkSurfaceKHR surface,
 * uint32_t imageCount, VkFormat format, VkColorSpaceKHR colorSpace, VkExtent2D
 * extent, uint32_t imageArrayLayers, VkImageUsageFlags usageFlags,
 *                                                  QueueFamilyIndices indices,
 *                                                  VkSurfaceTransformFlagBitsKHR
 * currentTransform, VkPresentModeKHR presentMode) \brief Creates a Vulkan
 * swapchain create info structure.
 *
 * \brief This function creates a VkSwapchainCreateInfoKHR structure with the
 * specified parameters.
 *
 * \param surface The Vulkan surface associated with the swapchain.
 * \param imageCount The number of images in the swapchain.
 * \param format The format of the swapchain images.
 * \param colorSpace The color space of the swapchain images.
 * \param extent The extent (width and height) of the swapchain images.
 * \param imageArrayLayers The number of layers for each image in the swapchain.
 * \param usageFlags The usage flags for the swapchain images.
 * \param indices The queue family indices for the swapchain.
 * \param currentTransform The transform flag for the swapchain images.
 * \param presentMode The presentation mode for presenting swapchain images.
 *
 * \return The created VkSwapchainCreateInfoKHR structure.
 */
VkSwapchainCreateInfoKHR
swapChainCreateInfo(VkSurfaceKHR surface, uint32_t imageCount, VkFormat format,
                    VkColorSpaceKHR colorSpace, VkExtent2D extent,
                    uint32_t imageArrayLayers, VkImageUsageFlags usageFlags,
                    QueueFamilyIndices indices,
                    VkSurfaceTransformFlagBitsKHR currentTransform,
                    VkPresentModeKHR presentMode);

/**
 * \fn VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(VkDescriptorType
 * type, uint32_t poolSizeInt) \brief Creates a Vulkan descriptor pool create
 * info structure.
 *
 * \brief This function creates a VkDescriptorPoolCreateInfo structure with the
 * specified parameters.
 *
 * \param type The type of descriptor stored in the pool.
 * \param poolSizeInt The number of descriptors in the pool.
 *
 * \return The created VkDescriptorPoolCreateInfo structure.
 */
VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(VkDescriptorType type,
                                                    uint32_t poolSizeInt);

/**
 * \fn VkDescriptorSetAllocateInfo descriptorSetAllocInfo(VkDescriptorPool
 * descriptorPool, std::vector<VkDescriptorSetLayout> &layouts) \brief Creates a
 * Vulkan descriptor set allocate info structure.
 *
 * \brief This function creates a VkDescriptorSetAllocateInfo structure with the
 * specified parameters.
 *
 * \param descriptorPool The Vulkan descriptor pool from which to allocate
 * descriptor sets. \param layouts The vector of descriptor set layouts to
 * allocate from.
 *
 * \return The created VkDescriptorSetAllocateInfo structure.
 */
VkDescriptorSetAllocateInfo
descriptorSetAllocInfo(VkDescriptorPool descriptorPool,
                       std::vector<VkDescriptorSetLayout> &layouts);

/**
 * \fn VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(VkDescriptorType
 * descType, VkShaderStageFlags stageFlags, uint32_t binding, uint32_t
 * descriptors) \brief Creates a Vulkan descriptor set layout binding.
 *
 * \brief This function creates a VkDescriptorSetLayoutBinding structure with
 * the specified parameters.
 *
 * \param descType The type of the descriptors in the binding.
 * \param stageFlags The shader stages that can access this binding.
 * \param binding The binding number of the descriptor set layout.
 * \param descriptors The number of descriptors in the binding.
 *
 * \return The created VkDescriptorSetLayoutBinding structure.
 */
VkDescriptorSetLayoutBinding
descriptorSetLayoutBinding(VkDescriptorType descType,
                           VkShaderStageFlags stageFlags, uint32_t binding,
                           uint32_t descriptors = 1);

/**
 * \fn VkDescriptorSetLayoutCreateInfo
 * descriptorSetLayoutInfo(VkDescriptorSetLayoutBinding *bindings, uint32_t
 * bind_count) \brief Creates a Vulkan descriptor set layout create info
 * structure.
 *
 * \brief This function creates a VkDescriptorSetLayoutCreateInfo structure with
 * the specified parameters.
 *
 * \param bindings An array of VkDescriptorSetLayoutBinding structures
 * describing the layout of the descriptor set. \param bind_count The number of
 * elements in the bindings array.
 *
 * \return The created VkDescriptorSetLayoutCreateInfo structure.
 */
VkDescriptorSetLayoutCreateInfo
descriptorSetLayoutInfo(VkDescriptorSetLayoutBinding *bindings,
                        uint32_t bind_count);

/**
 * \fn VkImageCreateInfo imageCreateInfo(VkImageType type, VkFormat format,
 * VkImageUsageFlags usageFlags)
 *
 * \brief Create a default VkImageCreateInfo object
 *
 * \param type the image type (e.g. VK_IMAGE_TYPE_2D)
 * \param format The image format
 * \usageFlags the image usage flags (e. g. use for texture)
 *
 * \return A default image create info
 */
VkImageCreateInfo imageCreateInfo(VkImageType type, VkFormat format,
                                  VkImageUsageFlags usageFlags);

/**
 * \fn VkSamplerCreateInfo samplerCreateInfo(VkFilter filter)
 *
 * \brief Returns a basic samplerCreateInfo
 */
VkSamplerCreateInfo samplerCreateInfo(VkFilter filter);

/**
 * \fn VkBufferImageCopy bufferImageCopy()
 *
 * \brief Returns a basic image copy info object
 */
VkBufferImageCopy bufferImageCopy(uint32_t width, uint32_t height);

/**
 *  \fn VkVertexInputAttributeDescription
 * vertexInputAttributeDescription(uint32_t binding, uint32_t location,
                                VkFormat format, uint32_t offset);
 * \biref Create a vertex attribute descriptor (essentially a description of the
 vertex struct field)
 */

VkVertexInputAttributeDescription
vertexInputAttributeDescription(uint32_t binding, uint32_t location,
                                VkFormat format, uint32_t offset);

/**
 * \fn VkVertexInputBindingDescription
vertexInputBindingDescription(uint32_t binding, uint32_t stride,
                              VkVertexInputRate inputRate);
 * \biref Create a vertex Input binding (essentially a description of the
 whole vertex struct)
 */
VkVertexInputBindingDescription
vertexInputBindingDescription(uint32_t binding, uint32_t stride,
                              VkVertexInputRate inputRate);

/**
* \fn VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet,
                                     VkDescriptorType type, uint32_t binding,
                                     VkDescriptorImageInfo *imageInfo,
                                     uint32_t descriptorCount = 1);
 * \ create a descriptor set
*/
VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet,
                                        VkDescriptorType type, uint32_t binding,
                                        VkDescriptorImageInfo *imageInfo,
                                        uint32_t descriptorCount = 1);

/**
* \fn VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet,
                                     VkDescriptorType type, uint32_t binding,
                                     VkDescriptorBufferInfo *imageInfo,
                                     uint32_t descriptorCount = 1);
 * \ create a descriptor set
*/
VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet,
                                        VkDescriptorType type, uint32_t binding,
                                        VkDescriptorBufferInfo *bufferInfo,
                                        uint32_t descriptorCount = 1);

VkDescriptorSetLayoutCreateInfo
descriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutBinding *pBindings,
                              uint32_t bindingCount);

VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
    const std::vector<VkDescriptorSetLayoutBinding> &bindings);

VkPipelineLayoutCreateInfo
pipelineLayoutCreateInfo(const VkDescriptorSetLayout *pSetLayouts,
                         uint32_t setLayoutCount = 1);
} // namespace create_info
