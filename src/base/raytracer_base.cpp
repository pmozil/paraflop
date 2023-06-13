#include "vulkan_utils/raytracer_base.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/utils.hpp"

namespace raytracer {
void RaytracerBase::updateRenderPass() {

    vkDestroyRenderPass(*m_deviceHandler, m_swapChain->getRenderPass(),
                        nullptr);

    std::array<VkAttachmentDescription, 2> attachments = {};
    // Color attachment
    attachments[0].format = m_swapChain->swapChainImageFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Depth attachment
    attachments[1].format = m_swapChain->depthBuffer.format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.pResolveAttachments = nullptr;

    // Subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();
    VK_CHECK(vkCreateRenderPass(*m_deviceHandler, &renderPassInfo, nullptr,
                                m_swapChain->renderPasses.data()));
}

RaytracerBase::ScratchBuffer
RaytracerBase::createScratchBuffer(VkDeviceSize size) {
    ScratchBuffer scratchBuffer{};
    // Buffer and memory
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                             VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VK_CHECK(vkCreateBuffer(*m_deviceHandler, &bufferCreateInfo, nullptr,
                            &scratchBuffer.handle));
    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(*m_deviceHandler, scratchBuffer.handle,
                                  &memoryRequirements);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
    memoryAllocateFlagsInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = m_deviceHandler->getMemoryType(
        memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memoryAllocateInfo, nullptr,
                              &scratchBuffer.memory));
    VK_CHECK(vkBindBufferMemory(*m_deviceHandler, scratchBuffer.handle,
                                scratchBuffer.memory, 0));
    // Buffer device address
    VkBufferDeviceAddressInfoKHR bufferDeviceAddresInfo{};
    bufferDeviceAddresInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAddresInfo.buffer = scratchBuffer.handle;
    scratchBuffer.deviceAddress =
        vkGetBufferDeviceAddressKHR(*m_deviceHandler, &bufferDeviceAddresInfo);
    return scratchBuffer;
}

void RaytracerBase::deleteScratchBuffer(ScratchBuffer &scratchBuffer) {
    if (scratchBuffer.memory != VK_NULL_HANDLE) {
        vkFreeMemory(*m_deviceHandler, scratchBuffer.memory, nullptr);
    }
    if (scratchBuffer.handle != VK_NULL_HANDLE) {
        vkDestroyBuffer(*m_deviceHandler, scratchBuffer.handle, nullptr);
    }
}

void RaytracerBase::createAccelerationStructure(
    AccelerationStructure &accelerationStructure,
    VkAccelerationStructureTypeKHR type,
    VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo) {
    // Buffer and memory
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
    bufferCreateInfo.usage =
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VK_CHECK(vkCreateBuffer(*m_deviceHandler, &bufferCreateInfo, nullptr,
                            &accelerationStructure.buffer));

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(
        *m_deviceHandler, accelerationStructure.buffer, &memoryRequirements);
    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
    memoryAllocateFlagsInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = m_deviceHandler->getMemoryType(
        memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memoryAllocateInfo, nullptr,
                              &accelerationStructure.memory));
    VK_CHECK(vkBindBufferMemory(*m_deviceHandler, accelerationStructure.buffer,
                                accelerationStructure.memory, 0));

    // Acceleration structure
    VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info{};
    accelerationStructureCreate_info.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationStructureCreate_info.buffer = accelerationStructure.buffer;
    accelerationStructureCreate_info.size =
        buildSizeInfo.accelerationStructureSize;
    accelerationStructureCreate_info.type = type;
    vkCreateAccelerationStructureKHR(*m_deviceHandler,
                                     &accelerationStructureCreate_info, nullptr,
                                     &accelerationStructure.handle);
    // AS device address
    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
    accelerationDeviceAddressInfo.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    accelerationDeviceAddressInfo.accelerationStructure =
        accelerationStructure.handle;
    accelerationStructure.deviceAddress =
        vkGetAccelerationStructureDeviceAddressKHR(
            *m_deviceHandler, &accelerationDeviceAddressInfo);
}

void RaytracerBase::deleteAccelerationStructure(
    AccelerationStructure &accelerationStructure) {
    vkFreeMemory(*m_deviceHandler, accelerationStructure.memory, nullptr);
    vkDestroyBuffer(*m_deviceHandler, accelerationStructure.buffer, nullptr);
    vkDestroyAccelerationStructureKHR(*m_deviceHandler,
                                      accelerationStructure.handle, nullptr);
}

uint64_t RaytracerBase::getBufferDeviceAddress(VkBuffer buffer) {
    VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
    bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = buffer;

    return vkGetBufferDeviceAddressKHR(*m_deviceHandler, &bufferDeviceAI);
}

void RaytracerBase::createStorageImage(VkFormat format, VkExtent3D extent) {
    // Release ressources if image is to be recreated
    if (storageImage.image != VK_NULL_HANDLE) {
        vkDestroyImageView(*m_deviceHandler, storageImage.view, nullptr);
        vkDestroyImage(*m_deviceHandler, storageImage.image, nullptr);
        vkFreeMemory(*m_deviceHandler, storageImage.memory, nullptr);
        storageImage = {};
    }

    VkImageCreateInfo image = create_info::imageCreateInfo(
        VK_IMAGE_TYPE_2D, format,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    image.extent = extent;
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VK_CHECK(
        vkCreateImage(*m_deviceHandler, &image, nullptr, &storageImage.image));

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(*m_deviceHandler, storageImage.image,
                                 &memReqs);
    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memReqs.size;
    memoryAllocateInfo.memoryTypeIndex = m_deviceHandler->getMemoryType(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memoryAllocateInfo, nullptr,
                              &storageImage.memory));
    VK_CHECK(vkBindImageMemory(*m_deviceHandler, storageImage.image,
                               storageImage.memory, 0));

    VkImageViewCreateInfo colorImageView{};
    colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorImageView.format = format;
    colorImageView.subresourceRange = {};
    colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageView.subresourceRange.baseMipLevel = 0;
    colorImageView.subresourceRange.levelCount = 1;
    colorImageView.subresourceRange.baseArrayLayer = 0;
    colorImageView.subresourceRange.layerCount = 1;
    colorImageView.image = storageImage.image;
    VK_CHECK(vkCreateImageView(*m_deviceHandler, &colorImageView, nullptr,
                               &storageImage.view));

    VkCommandBuffer cmdBuffer = m_commandBuffer->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    utils::setImageLayout(cmdBuffer, storageImage.image,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    m_commandBuffer->flushCommandBuffer(cmdBuffer,
                                        m_deviceHandler->getTransferQueue());
}

void RaytracerBase::deleteStorageImage() {
    vkDestroyImageView(*m_deviceHandler, storageImage.view, nullptr);
    vkDestroyImage(*m_deviceHandler, storageImage.image, nullptr);
    vkFreeMemory(*m_deviceHandler, storageImage.memory, nullptr);
}

void RaytracerBase::prepare() {
    rayTracingPipelineProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    VkPhysicalDeviceProperties2 deviceProperties2{};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties2.pNext = &rayTracingPipelineProperties;
    vkGetPhysicalDeviceProperties2(m_deviceHandler->physicalDevice,
                                   &deviceProperties2);
    accelerationStructureFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &accelerationStructureFeatures;
    vkGetPhysicalDeviceFeatures2(m_deviceHandler->physicalDevice,
                                 &deviceFeatures2);
    // Get the function pointers required for ray tracing
    vkGetBufferDeviceAddressKHR =
        reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(
            *m_deviceHandler, "vkGetBufferDeviceAddressKHR"));
    vkCmdBuildAccelerationStructuresKHR =
        reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
            vkGetDeviceProcAddr(*m_deviceHandler,
                                "vkCmdBuildAccelerationStructuresKHR"));
    vkBuildAccelerationStructuresKHR =
        reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(
            vkGetDeviceProcAddr(*m_deviceHandler,
                                "vkBuildAccelerationStructuresKHR"));
    vkCreateAccelerationStructureKHR =
        reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
            vkGetDeviceProcAddr(*m_deviceHandler,
                                "vkCreateAccelerationStructureKHR"));
    vkDestroyAccelerationStructureKHR =
        reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
            vkGetDeviceProcAddr(*m_deviceHandler,
                                "vkDestroyAccelerationStructureKHR"));
    vkGetAccelerationStructureBuildSizesKHR =
        reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
            vkGetDeviceProcAddr(*m_deviceHandler,
                                "vkGetAccelerationStructureBuildSizesKHR"));
    vkGetAccelerationStructureDeviceAddressKHR =
        reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
            vkGetDeviceProcAddr(*m_deviceHandler,
                                "vkGetAccelerationStructureDeviceAddressKHR"));
    vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
        vkGetDeviceProcAddr(*m_deviceHandler, "vkCmdTraceRaysKHR"));
    vkGetRayTracingShaderGroupHandlesKHR =
        reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
            vkGetDeviceProcAddr(*m_deviceHandler,
                                "vkGetRayTracingShaderGroupHandlesKHR"));
    vkCreateRayTracingPipelinesKHR =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
            vkGetDeviceProcAddr(*m_deviceHandler,
                                "vkCreateRayTracingPipelinesKHR"));
    updateRenderPass();
}

VkStridedDeviceAddressRegionKHR
RaytracerBase::getSbtEntryStridedDeviceAddressRegion(VkBuffer buffer,
                                                     uint32_t handleCount) {
    const uint32_t handleSizeAligned = utils::alignedSize(
        rayTracingPipelineProperties.shaderGroupHandleSize,
        rayTracingPipelineProperties.shaderGroupHandleAlignment);
    VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegionKHR{};
    stridedDeviceAddressRegionKHR.deviceAddress =
        getBufferDeviceAddress(buffer);
    stridedDeviceAddressRegionKHR.stride = handleSizeAligned;
    stridedDeviceAddressRegionKHR.size = handleCount * handleSizeAligned;
    return stridedDeviceAddressRegionKHR;
}

void RaytracerBase::createShaderBindingTable(
    ShaderBindingTable &shaderBindingTable, uint32_t handleCount) {
    // Create buffer to hold all shader handles for the SBT
    VK_CHECK(m_deviceHandler->createBuffer(
        VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        rayTracingPipelineProperties.shaderGroupHandleSize * handleCount,
        &shaderBindingTable.buffer, &shaderBindingTable.memory, nullptr));
    // Get the strided address to be used when dispatching the rays
    shaderBindingTable.stridedDeviceAddressRegion =
        getSbtEntryStridedDeviceAddressRegion(shaderBindingTable.buffer,
                                              handleCount);
    // Map persistent
    shaderBindingTable.map();
}

VkPipelineShaderStageCreateInfo
RaytracerBase::loadShader(std::string &fileName, VkShaderStageFlagBits stage) {
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
    shaderStage.module = utils::loadShader(fileName.c_str(), *m_deviceHandler);
    shaderStage.pName = "main";
    assert(shaderStage.module != VK_NULL_HANDLE);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}

VkPipelineShaderStageCreateInfo
RaytracerBase::loadShader(std::string fileName, VkShaderStageFlagBits stage) {
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
    shaderStage.module = utils::loadShader(fileName.c_str(), *m_deviceHandler);
    shaderStage.pName = "main";
    assert(shaderStage.module != VK_NULL_HANDLE);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}
} // namespace raytracer
