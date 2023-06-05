/**
 * Most iof this code has been taken from Sacha Willems' Samples.
 */
#include "vulkan_utils/texture.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/utils.hpp"

namespace texture {
void Texture::updateDescriptor() {
    descriptor.sampler = sampler;
    descriptor.imageView = view;
    descriptor.imageLayout = imageLayout;
}

void Texture::destroy() {
    vkDestroyImageView(*m_deviceHandler, view, nullptr);
    vkDestroyImage(*m_deviceHandler, image, nullptr);
    if (sampler != nullptr) {
        vkDestroySampler(*m_deviceHandler, sampler, nullptr);
    }
    vkFreeMemory(*m_deviceHandler, deviceMemory, nullptr);
}

ktxResult Texture::loadKTXFile(const std::string &filename,
                               ktxTexture **target) {
    if (utils::fileExists(filename)) {
        utils::exitFatal("Could not load texture from " + filename +
                             "\n\nMake sure the assets submodule has been "
                             "checked out and is up-to-date.",
                         -1);
    }
    ktxResult result = KTX_SUCCESS;
    result = ktxTexture_CreateFromNamedFile(
        filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);
    return result;
}

void Texture2D::createImageWithStaging(ktxTexture *ktxTexture,
                                       ktx_uint8_t *ktxTextureData,
                                       ktx_size_t ktxTextureSize,
                                       VkFormat format,
                                       VkImageUsageFlags imageUsageFlags) {

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;

    // Use a separate command buffer for texture loading
    VkCommandBuffer copyCmd = m_commandBufferHandler->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // Create a host-visible staging buffer that contains the raw image data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = ktxTextureSize;
    // This buffer is used as a transfer source for the buffer copy
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(*m_deviceHandler, &bufferCreateInfo, nullptr,
                            &stagingBuffer));

    // Get memory requirements for the staging buffer (alignment, memory
    // type bits)
    vkGetBufferMemoryRequirements(*m_deviceHandler, stagingBuffer, &memReqs);

    memAllocInfo.allocationSize = memReqs.size;
    // Get memory type index for a host visible buffer
    memAllocInfo.memoryTypeIndex = m_deviceHandler->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memAllocInfo, nullptr,
                              &stagingMemory));
    VK_CHECK(
        vkBindBufferMemory(*m_deviceHandler, stagingBuffer, stagingMemory, 0));

    // Copy texture data into staging buffer
    uint8_t *data;
    VK_CHECK(vkMapMemory(*m_deviceHandler, stagingMemory, 0, memReqs.size, 0,
                         (void **)&data));
    memcpy(data, ktxTextureData, ktxTextureSize);
    vkUnmapMemory(*m_deviceHandler, stagingMemory);

    // Setup buffer copy regions for each mip level
    std::vector<VkBufferImageCopy> bufferCopyRegions;

    for (uint32_t i = 0; i < mipLevels; i++) {
        ktx_size_t offset;
        KTX_error_code result =
            ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
        assert(result == KTX_SUCCESS);

        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = i;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width =
            std::max(1U, ktxTexture->baseWidth >> i);
        bufferCopyRegion.imageExtent.height =
            std::max(1U, ktxTexture->baseHeight >> i);
        bufferCopyRegion.imageExtent.depth = 1;
        bufferCopyRegion.bufferOffset = offset;

        bufferCopyRegions.push_back(bufferCopyRegion);
    }

    // Create optimal tiled target image
    VkImageCreateInfo imageCreateInfo =
        create_info::imageCreateInfo(VK_IMAGE_TYPE_2D, format, imageUsageFlags);
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.extent = {width, height, 1};
    // Ensure that the TRANSFER_DST bit is set for staging
    if (!static_cast<bool>(imageCreateInfo.usage &
                           VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    VK_CHECK(
        vkCreateImage(*m_deviceHandler, &imageCreateInfo, nullptr, &image));

    vkGetImageMemoryRequirements(*m_deviceHandler, image, &memReqs);

    memAllocInfo.allocationSize = memReqs.size;

    memAllocInfo.memoryTypeIndex = m_deviceHandler->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memAllocInfo, nullptr,
                              &deviceMemory));
    VK_CHECK(vkBindImageMemory(*m_deviceHandler, image, deviceMemory, 0));

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipLevels;
    subresourceRange.layerCount = 1;

    // Image barrier for optimal image (target)
    // Optimal image will be used as destination for the copy
    utils::setImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          subresourceRange);

    // Copy mip levels from staging buffer
    vkCmdCopyBufferToImage(copyCmd, stagingBuffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(bufferCopyRegions.size()),
                           bufferCopyRegions.data());

    // Change texture image layout to shader read after all mip levels have
    // been copied
    this->imageLayout = imageLayout;
    utils::setImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          imageLayout, subresourceRange);

    m_commandBufferHandler->flushCommandBuffer(
        copyCmd, m_deviceHandler->getTransferQueue());

    // Clean up staging resources
    vkFreeMemory(*m_deviceHandler, stagingMemory, nullptr);
    vkDestroyBuffer(*m_deviceHandler, stagingBuffer, nullptr);
}

void Texture2D::createImageWithoutStaging(ktx_uint8_t *ktxTextureData,
                                          VkFormat format,
                                          VkImageUsageFlags imageUsageFlags,
                                          VkImageLayout layout) {
    // Get device properties for the requested texture format
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_deviceHandler->physicalDevice, format,
                                        &formatProperties);

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;

    // Use a separate command buffer for texture loading
    VkCommandBuffer copyCmd = m_commandBufferHandler->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // Prefer using optimal tiling, as linear tiling
    // may support only a small set of features
    // depending on implementation (e.g. no mip maps, only one layer, etc.)

    // Check if this support is supported for linear tiling
    assert(formatProperties.linearTilingFeatures &
           VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

    VkImage mappableImage;
    VkDeviceMemory mappableMemory;

    VkImageCreateInfo imageCreateInfo =
        create_info::imageCreateInfo(VK_IMAGE_TYPE_2D, format, imageUsageFlags);
    imageCreateInfo.extent = {width, height, 1};

    // Load mip map level 0 to linear tiling image
    VK_CHECK(vkCreateImage(*m_deviceHandler, &imageCreateInfo, nullptr,
                           &mappableImage));

    // Get memory requirements for this image
    // like size and alignment
    vkGetImageMemoryRequirements(*m_deviceHandler, mappableImage, &memReqs);
    // Set memory allocation size to required memory size
    memAllocInfo.allocationSize = memReqs.size;

    // Get memory type that can be mapped to host memory
    memAllocInfo.memoryTypeIndex = m_deviceHandler->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Allocate host memory
    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memAllocInfo, nullptr,
                              &mappableMemory));

    // Bind allocated image for use
    VK_CHECK(
        vkBindImageMemory(*m_deviceHandler, mappableImage, mappableMemory, 0));

    // Get sub resource layout
    // Mip map count, array layer, etc.
    VkImageSubresource subRes = {};
    subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subRes.mipLevel = 0;

    VkSubresourceLayout subResLayout;
    void *data;

    // Get sub resources layout
    // Includes row pitch, size offsets, etc.
    vkGetImageSubresourceLayout(*m_deviceHandler, mappableImage, &subRes,
                                &subResLayout);

    // Map image memory
    VK_CHECK(vkMapMemory(*m_deviceHandler, mappableMemory, 0, memReqs.size, 0,
                         &data));

    // Copy image data into memory
    memcpy(data, ktxTextureData, memReqs.size);

    vkUnmapMemory(*m_deviceHandler, mappableMemory);

    // Linear tiled images don't need to be staged
    // and can be directly used as textures
    image = mappableImage;
    deviceMemory = mappableMemory;
    this->imageLayout = layout;

    // Setup image memory barrier
    utils::setImageLayout(copyCmd, image, VK_IMAGE_ASPECT_COLOR_BIT,
                          VK_IMAGE_LAYOUT_UNDEFINED, imageLayout);

    m_commandBufferHandler->flushCommandBuffer(
        copyCmd, m_deviceHandler->getTransferQueue());
}

Texture2D::Texture2D(
    const std::string &filename, VkFormat format,
    std::shared_ptr<device::DeviceHandler> deviceHandler,
    std::shared_ptr<command_buffer::CommandBufferHandler> commandBufferHandler,
    VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout,
    bool forceLinear) {
    ktxTexture *ktxTexture;
    ktxResult result = loadKTXFile(filename, &ktxTexture);
    assert(result == KTX_SUCCESS);

    m_deviceHandler = std::move(deviceHandler);
    m_commandBufferHandler = std::move(commandBufferHandler);
    width = ktxTexture->baseWidth;
    height = ktxTexture->baseHeight;
    mipLevels = ktxTexture->numLevels;

    ktx_uint8_t *ktxTextureData = ktxTexture_GetData(ktxTexture);
    // ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);
    ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);

    // Get device properties for the requested texture format
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_deviceHandler->physicalDevice, format,
                                        &formatProperties);

    // Only use linear tiling if requested (and supported by the device)
    // Support for linear tiling is mostly limited, so prefer to use
    // optimal tiling instead
    // On most implementations linear tiling will only support a very
    // limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
    auto useStaging = static_cast<VkBool32>(!forceLinear);

    // Use a separate command buffer for texture loading
    if (static_cast<bool>(useStaging)) {
        createImageWithStaging(ktxTexture, ktxTextureData, ktxTextureSize,
                               format, imageUsageFlags);
    } else {
        createImageWithoutStaging(ktxTextureData, format, imageUsageFlags,
                                  imageLayout);
    }

    ktxTexture_Destroy(ktxTexture);

    // Create a default sampler
    VkSamplerCreateInfo samplerCreateInfo =
        create_info::samplerCreateInfo(VK_FILTER_LINEAR);
    // Max level-of-detail should match mip level count
    samplerCreateInfo.maxLod =
        static_cast<bool>(useStaging) ? (float)mipLevels : 0.0F;
    // Only enable anisotropic filtering if enabled on the device
    samplerCreateInfo.maxAnisotropy =
        static_cast<bool>(m_deviceHandler->enabledFeatures.samplerAnisotropy)
            ? m_deviceHandler->properties.limits.maxSamplerAnisotropy
            : 1.0F;
    samplerCreateInfo.anisotropyEnable =
        m_deviceHandler->enabledFeatures.samplerAnisotropy;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK(vkCreateSampler(*m_deviceHandler, &samplerCreateInfo, nullptr,
                             &sampler));

    // Create image view
    // Textures are not directly accessed by the shaders and
    // are abstracted by image views containing additional
    // information and sub resource ranges
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    // Linear tiling usually won't support mip maps
    // Only set mip map count if optimal tiling is used
    viewCreateInfo.subresourceRange.levelCount =
        static_cast<bool>(useStaging) ? mipLevels : 1;
    viewCreateInfo.image = image;
    VK_CHECK(
        vkCreateImageView(*m_deviceHandler, &viewCreateInfo, nullptr, &view));

    // Update descriptor image info member that can be used for setting up
    // descriptor sets
    updateDescriptor();
}

/**
 * Creates a 2D texture from a buffer
 *
 * @param buffer Buffer containing texture data to upload
 * @param bufferSize Size of the buffer in machine units
 * @param width Width of the texture to create
 * @param height Height of the texture to create
 * @param format Vulkan format of the image data stored in the file
 * @param device Vulkan device to create the texture on
 * @param m_deviceHandler->getTransferQueue() Queue used for the texture staging
 * copy commands (must support transfer)
 * @param (Optional) filter Texture filtering for the sampler (defaults to
 * VK_FILTER_LINEAR)
 * @param (Optional) imageUsageFlags Usage flags for the texture's image
 * (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
 * @param (Optional) imageLayout Usage layout for the texture (defaults
 * VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
 */
Texture2D::Texture2D(void *buffer, VkDeviceSize bufferSize, VkFormat format,
                     uint32_t texWidth, uint32_t texHeight,
                     std::shared_ptr<device::DeviceHandler> device,
                     std::shared_ptr<command_buffer::CommandBufferHandler>
                         m_commandBufferHandler,
                     VkFilter filter, VkImageUsageFlags imageUsageFlags,
                     VkImageLayout imageLayout) {
    assert(buffer);

    m_deviceHandler = std::move(device);
    this->m_commandBufferHandler = std::move(m_commandBufferHandler);
    width = texWidth;
    height = texHeight;
    mipLevels = 1;

    VkMemoryAllocateInfo memAllocInfo{};
    VkMemoryRequirements memReqs;

    // Use a separate command buffer for texture loading
    VkCommandBuffer copyCmd = m_commandBufferHandler->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // Create a host-visible staging buffer that contains the raw image data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = bufferSize;
    // This buffer is used as a transfer source for the buffer copy
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(*m_deviceHandler, &bufferCreateInfo, nullptr,
                            &stagingBuffer));

    // Get memory requirements for the staging buffer (alignment, memory type
    // bits)
    vkGetBufferMemoryRequirements(*m_deviceHandler, stagingBuffer, &memReqs);

    memAllocInfo.allocationSize = memReqs.size;
    // Get memory type index for a host visible buffer
    memAllocInfo.memoryTypeIndex = device->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memAllocInfo, nullptr,
                              &stagingMemory));
    VK_CHECK(
        vkBindBufferMemory(*m_deviceHandler, stagingBuffer, stagingMemory, 0));

    // Copy texture data into staging buffer
    uint8_t *data;
    VK_CHECK(vkMapMemory(*m_deviceHandler, stagingMemory, 0, memReqs.size, 0,
                         (void **)&data));
    memcpy(data, buffer, bufferSize);
    vkUnmapMemory(*m_deviceHandler, stagingMemory);

    VkBufferImageCopy bufferCopyRegion =
        create_info::bufferImageCopy(width, height);

    // Create optimal tiled target image
    VkImageCreateInfo imageCreateInfo =
        create_info::imageCreateInfo(VK_IMAGE_TYPE_2D, format, imageUsageFlags);
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.extent = {width, height, 1};
    // Ensure that the TRANSFER_DST bit is set for staging
    if (!static_cast<bool>(imageCreateInfo.usage &
                           VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    VK_CHECK(
        vkCreateImage(*m_deviceHandler, &imageCreateInfo, nullptr, &image));

    vkGetImageMemoryRequirements(*m_deviceHandler, image, &memReqs);

    memAllocInfo.allocationSize = memReqs.size;

    memAllocInfo.memoryTypeIndex = device->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memAllocInfo, nullptr,
                              &deviceMemory));
    VK_CHECK(vkBindImageMemory(*m_deviceHandler, image, deviceMemory, 0));

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipLevels;
    subresourceRange.layerCount = 1;

    // Image barrier for optimal image (target)
    // Optimal image will be used as destination for the copy
    utils::setImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          subresourceRange);

    // Copy mip levels from staging buffer
    vkCmdCopyBufferToImage(copyCmd, stagingBuffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &bufferCopyRegion);

    // Change texture image layout to shader read after all mip levels have been
    // copied
    this->imageLayout = imageLayout;
    utils::setImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          imageLayout, subresourceRange);

    m_commandBufferHandler->flushCommandBuffer(
        copyCmd, m_deviceHandler->getTransferQueue());

    // Clean up staging resources
    vkFreeMemory(*m_deviceHandler, stagingMemory, nullptr);
    vkDestroyBuffer(*m_deviceHandler, stagingBuffer, nullptr);

    // Create sampler
    VkSamplerCreateInfo samplerCreateInfo =
        create_info::samplerCreateInfo(filter);
    VK_CHECK(vkCreateSampler(*m_deviceHandler, &samplerCreateInfo, nullptr,
                             &sampler));

    // Create image view
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext = nullptr;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.image = image;
    VK_CHECK(
        vkCreateImageView(*m_deviceHandler, &viewCreateInfo, nullptr, &view));

    // Update descriptor image info member that can be used for setting up
    // descriptor sets
    updateDescriptor();
}

/**
 * Load a 2D texture array including all mip levels
 *
 * @param filename File to load (supports .ktx)
 * @param format Vulkan format of the image data stored in the file
 * @param device Vulkan device to create the texture on
 * @param m_deviceHandler->getTransferQueue() Queue used for the texture staging
 * copy commands (must support transfer)
 * @param (Optional) imageUsageFlags Usage flags for the texture's image
 * (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
 * @param (Optional) imageLayout Usage layout for the texture (defaults
 * VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
 *
 */
Texture2DArray::Texture2DArray(
    const std::string &filename, VkFormat format,
    std::shared_ptr<device::DeviceHandler> m_deviceHandler,
    std::shared_ptr<command_buffer::CommandBufferHandler>
        m_commandBufferHandler,
    VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout) {
    ktxTexture *ktxTexture;
    ktxResult result = loadKTXFile(filename, &ktxTexture);
    assert(result == KTX_SUCCESS);

    this->m_deviceHandler = std::move(m_deviceHandler);
    this->m_commandBufferHandler = std::move(m_commandBufferHandler);
    width = ktxTexture->baseWidth;
    height = ktxTexture->baseHeight;
    layerCount = ktxTexture->numLayers;
    mipLevels = ktxTexture->numLevels;

    ktx_uint8_t *ktxTextureData = ktxTexture_GetData(ktxTexture);
    ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;

    // Create a host-visible staging buffer that contains the raw image data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = ktxTextureSize;
    // This buffer is used as a transfer source for the buffer copy
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(*m_deviceHandler, &bufferCreateInfo, nullptr,
                            &stagingBuffer));

    // Get memory requirements for the staging buffer (alignment, memory type
    // bits)
    vkGetBufferMemoryRequirements(*m_deviceHandler, stagingBuffer, &memReqs);

    memAllocInfo.allocationSize = memReqs.size;
    // Get memory type index for a host visible buffer
    memAllocInfo.memoryTypeIndex = m_deviceHandler->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memAllocInfo, nullptr,
                              &stagingMemory));
    VK_CHECK(
        vkBindBufferMemory(*m_deviceHandler, stagingBuffer, stagingMemory, 0));

    // Copy texture data into staging buffer
    uint8_t *data;
    VK_CHECK(vkMapMemory(*m_deviceHandler, stagingMemory, 0, memReqs.size, 0,
                         (void **)&data));
    memcpy(data, ktxTextureData, ktxTextureSize);
    vkUnmapMemory(*m_deviceHandler, stagingMemory);

    // Setup buffer copy regions for each layer including all of its miplevels
    std::vector<VkBufferImageCopy> bufferCopyRegions;

    for (uint32_t layer = 0; layer < layerCount; layer++) {
        for (uint32_t level = 0; level < mipLevels; level++) {
            ktx_size_t offset;
            KTX_error_code result =
                ktxTexture_GetImageOffset(ktxTexture, level, layer, 0, &offset);
            assert(result == KTX_SUCCESS);

            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask =
                VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = level;
            bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = ktxTexture->baseWidth >> level;
            bufferCopyRegion.imageExtent.height =
                ktxTexture->baseHeight >> level;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = offset;

            bufferCopyRegions.push_back(bufferCopyRegion);
        }
    }

    // Create optimal tiled target image
    VkImageCreateInfo imageCreateInfo =
        create_info::imageCreateInfo(VK_IMAGE_TYPE_2D, format, imageUsageFlags);
    imageCreateInfo.extent = {width, height, 1};
    // Ensure that the TRANSFER_DST bit is set for staging
    if (!static_cast<bool>(imageCreateInfo.usage &
                           VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    imageCreateInfo.arrayLayers = layerCount;
    imageCreateInfo.mipLevels = mipLevels;

    VK_CHECK(
        vkCreateImage(*m_deviceHandler, &imageCreateInfo, nullptr, &image));

    vkGetImageMemoryRequirements(*m_deviceHandler, image, &memReqs);

    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = m_deviceHandler->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memAllocInfo, nullptr,
                              &deviceMemory));
    VK_CHECK(vkBindImageMemory(*m_deviceHandler, image, deviceMemory, 0));

    // Use a separate command buffer for texture loading
    VkCommandBuffer copyCmd = m_commandBufferHandler->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // Image barrier for optimal image (target)
    // Set initial layout for all array layers (faces) of the optimal (target)
    // tiled texture
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipLevels;
    subresourceRange.layerCount = layerCount;

    utils::setImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          subresourceRange);

    // Copy the layers and mip levels from the staging buffer to the optimal
    // tiled image
    vkCmdCopyBufferToImage(copyCmd, stagingBuffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(bufferCopyRegions.size()),
                           bufferCopyRegions.data());

    // Change texture image layout to shader read after all faces have been
    // copied
    this->imageLayout = imageLayout;
    utils::setImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          imageLayout, subresourceRange);

    m_commandBufferHandler->flushCommandBuffer(
        copyCmd, m_deviceHandler->getTransferQueue());

    // Create sampler
    VkSamplerCreateInfo samplerCreateInfo =
        create_info::samplerCreateInfo(VK_FILTER_LINEAR);
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
    samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
    samplerCreateInfo.maxAnisotropy =
        static_cast<bool>(m_deviceHandler->enabledFeatures.samplerAnisotropy)
            ? m_deviceHandler->properties.limits.maxSamplerAnisotropy
            : 1.0F;
    samplerCreateInfo.anisotropyEnable =
        m_deviceHandler->enabledFeatures.samplerAnisotropy;
    samplerCreateInfo.maxLod = (float)mipLevels;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK(vkCreateSampler(*m_deviceHandler, &samplerCreateInfo, nullptr,
                             &sampler));

    // Create image view
    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    viewCreateInfo.subresourceRange.layerCount = layerCount;
    viewCreateInfo.subresourceRange.levelCount = mipLevels;
    viewCreateInfo.image = image;
    VK_CHECK(
        vkCreateImageView(*m_deviceHandler, &viewCreateInfo, nullptr, &view));

    // Clean up staging resources
    ktxTexture_Destroy(ktxTexture);
    vkFreeMemory(*m_deviceHandler, stagingMemory, nullptr);
    vkDestroyBuffer(*m_deviceHandler, stagingBuffer, nullptr);

    // Update descriptor image info member that can be used for setting up
    // descriptor sets
    updateDescriptor();
}
} // namespace texture
