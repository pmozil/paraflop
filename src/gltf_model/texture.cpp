#include "gltf_model/texture.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/staging_buffer.hpp"
#include "vulkan_utils/utils.hpp"

/*
    glTF texture loading class
*/

void gltf_model::Texture::updateDescriptor() {
    descriptor.sampler = sampler;
    descriptor.imageView = view;
    descriptor.imageLayout = imageLayout;
}

void gltf_model::Texture::destroy() {
    if (deviceHandler) {
        vkDestroyImageView(*deviceHandler, view, nullptr);
        vkDestroyImage(*deviceHandler, image, nullptr);
        vkFreeMemory(*deviceHandler, deviceMemory, nullptr);
        vkDestroySampler(*deviceHandler, sampler, nullptr);
    }
}

void gltf_model::Texture::makeglTFImage(tinygltf::Image &gltfimage,
                                        VkFormat &format, VkQueue copyQueue) {

    unsigned char *buffer = nullptr;
    VkDeviceSize bufferSize = 0;
    bool deleteBuffer = false;
    if (gltfimage.component == 3) {
        // Most devices don't support RGB only on Vulkan so convert if
        // necessary
        // TODO: Check actual format support and transform only if required
        bufferSize = gltfimage.width * gltfimage.height * 4;
        buffer = new unsigned char[bufferSize];
        unsigned char *rgba = buffer;
        unsigned char *rgb = gltfimage.image.data();
        for (int i = 0; i < gltfimage.width * gltfimage.height; ++i) {
            for (int32_t j = 0; j < 3; ++j) {
                rgba[j] = rgb[j];
            }
            rgba += 4;
            rgb += 3;
        }
        deleteBuffer = true;
    } else {
        buffer = gltfimage.image.data();
        bufferSize = gltfimage.image.size();
    }

    format = VK_FORMAT_R8G8B8A8_UNORM;

    VkFormatProperties formatProperties;

    width = gltfimage.width;
    height = gltfimage.height;
    mipLevels =
        static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1.0);

    vkGetPhysicalDeviceFormatProperties(deviceHandler->physicalDevice, format,
                                        &formatProperties);
    assert(formatProperties.optimalTilingFeatures &
           VK_FORMAT_FEATURE_BLIT_SRC_BIT);
    assert(formatProperties.optimalTilingFeatures &
           VK_FORMAT_FEATURE_BLIT_DST_BIT);

    buffer::StagingBuffer buf =
        buffer::StagingBuffer(deviceHandler, commandBuffer, bufferSize);
    buf.copy(buffer, bufferSize);

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent = {width, height, 1};
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_SAMPLED_BIT;
    VK_CHECK(vkCreateImage(*deviceHandler, &imageCreateInfo, nullptr, &image));

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs{};

    vkGetImageMemoryRequirements(*deviceHandler, image, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = deviceHandler->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(*deviceHandler, &memAllocInfo, nullptr,
                              &deviceMemory));

    VK_CHECK(vkBindImageMemory(*deviceHandler, image, deviceMemory, 0));

    VkCommandBuffer copyCmd = commandBuffer->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    {
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                             0, nullptr, 1, &imageMemoryBarrier);
    }

    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel = 0;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = width;
    bufferCopyRegion.imageExtent.height = height;
    bufferCopyRegion.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(copyCmd, buf.buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &bufferCopyRegion);

    {
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                             0, nullptr, 1, &imageMemoryBarrier);
    }

    commandBuffer->flushCommandBuffer(copyCmd, copyQueue, true);

    makeBlits(subresourceRange, copyQueue);

    if (deleteBuffer) {
        delete[] buffer;
    }
}

void gltf_model::Texture::makeBlits(VkImageSubresourceRange &subresourceRange,
                                    VkQueue copyQueue) {

    // Generate the mip chain (glTF uses jpg and png, so we need to create
    // this manually)
    VkCommandBuffer blitCmd = commandBuffer->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    for (uint32_t i = 1; i < mipLevels; i++) {
        VkImageBlit imageBlit{};

        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.srcSubresource.mipLevel = i - 1;
        imageBlit.srcOffsets[1].x = int32_t(width >> (i - 1));
        imageBlit.srcOffsets[1].y = int32_t(height >> (i - 1));
        imageBlit.srcOffsets[1].z = 1;

        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.layerCount = 1;
        imageBlit.dstSubresource.mipLevel = i;
        imageBlit.dstOffsets[1].x = int32_t(width >> i);
        imageBlit.dstOffsets[1].y = int32_t(height >> i);
        imageBlit.dstOffsets[1].z = 1;

        VkImageSubresourceRange mipSubRange = {};
        mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        mipSubRange.baseMipLevel = i;
        mipSubRange.levelCount = 1;
        mipSubRange.layerCount = 1;

        {
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = mipSubRange;
            vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                                 0, nullptr, 1, &imageMemoryBarrier);
        }

        vkCmdBlitImage(blitCmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                       &imageBlit, VK_FILTER_LINEAR);

        {
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = mipSubRange;
            vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                                 0, nullptr, 1, &imageMemoryBarrier);
        }
    }

    subresourceRange.levelCount = mipLevels;
    imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    {
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                             0, nullptr, 1, &imageMemoryBarrier);
    }
    commandBuffer->flushCommandBuffer(blitCmd, copyQueue, true);
}

void gltf_model::Texture::makeKTXImage(const std::string &filename,
                                       VkFormat &format, VkQueue copyQueue) {

    ktxTexture *ktxTexture;

    ktxResult result = KTX_SUCCESS;
    if (!utils::fileExists(filename)) {
        utils::exitFatal("Could not load texture from " + filename +
                             "\n\nMake sure the assets submodule has "
                             "been checked out and is up-to-date.",
                         -1);
    }

    result = ktxTexture_CreateFromNamedFile(
        filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
    UNUSED(result);
    assert(result == KTX_SUCCESS);

    width = ktxTexture->baseWidth;
    height = ktxTexture->baseHeight;
    mipLevels = ktxTexture->numLevels;

    ktx_uint8_t *ktxTextureData = ktxTexture_GetData(ktxTexture);
    ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);
    // @todo: Use ktxTexture_GetVkFormat(ktxTexture)
    // format = ktxTexture_GetVkFormat(ktxTexture);
    format = VK_FORMAT_R8G8B8A8_UNORM;

    // Get device properties for the requested texture format

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(deviceHandler->physicalDevice, format,
                                        &formatProperties);

    VkCommandBuffer copyCmd = commandBuffer->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    buffer::StagingBuffer buf =
        buffer::StagingBuffer(deviceHandler, commandBuffer, ktxTextureSize);

    buf.copy(ktxTextureData, ktxTextureSize);
    buf.unmap();

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;

    std::vector<VkBufferImageCopy> bufferCopyRegions;
    for (uint32_t i = 0; i < mipLevels; i++) {
        ktx_size_t offset;
        KTX_error_code result =
            ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
        UNUSED(result);
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
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent = {width, height, 1};
    imageCreateInfo.usage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VK_CHECK(vkCreateImage(*deviceHandler, &imageCreateInfo, nullptr, &image));

    vkGetImageMemoryRequirements(*deviceHandler, image, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = deviceHandler->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(*deviceHandler, &memAllocInfo, nullptr,
                              &deviceMemory));
    VK_CHECK(vkBindImageMemory(*deviceHandler, image, deviceMemory, 0));

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipLevels;
    subresourceRange.layerCount = 1;

    utils::setImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          subresourceRange);
    vkCmdCopyBufferToImage(copyCmd, buf.buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(bufferCopyRegions.size()),
                           bufferCopyRegions.data());
    utils::setImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          subresourceRange);
    commandBuffer->flushCommandBuffer(copyCmd, copyQueue);
    this->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    ktxTexture_Destroy(ktxTexture);
}

void gltf_model::Texture::fromglTfImage(
    tinygltf::Image &gltfimage, std::string &path,
    std::shared_ptr<device::DeviceHandler> device,
    std::shared_ptr<command_buffer::CommandBufferHandler> cmdBuf,
    VkQueue copyQueue) {

    this->deviceHandler = std::move(device);
    this->commandBuffer = std::move(cmdBuf);

    bool isKtx = false;
    // Image points to an external ktx file
    if (gltfimage.uri.find_last_of('.') != std::string::npos) {
        if (gltfimage.uri.substr(gltfimage.uri.find_last_of('.') + 1) ==
            "ktx") {
            isKtx = true;
        }
    }

    VkFormat format;

    if (!isKtx) {
        // Texture was loaded using STB_Image
        makeglTFImage(gltfimage, format, copyQueue);
    } else {
        makeKTXImage(path + "/" + gltfimage.uri, format, copyQueue);
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.maxAnisotropy = 1.0;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxLod = (float)mipLevels;
    samplerInfo.maxAnisotropy = MAX_ANISOTROPY;
    samplerInfo.anisotropyEnable = VK_TRUE;
    VK_CHECK(vkCreateSampler(*deviceHandler, &samplerInfo, nullptr, &sampler));

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = mipLevels;
    VK_CHECK(vkCreateImageView(*deviceHandler, &viewInfo, nullptr, &view));

    descriptor.sampler = sampler;
    descriptor.imageView = view;
    descriptor.imageLayout = imageLayout;
}
