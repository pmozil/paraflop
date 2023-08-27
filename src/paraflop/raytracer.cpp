#include "raytracer.hpp"
#include "vulkan_utils/utils.hpp"

/*
    Create the bottom level acceleration structure contains the scene's actual
   geometry (vertices, triangles)
*/
void Raytracer::createBottomLevelAccelerationStructure() {
    VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
    VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};

    vertexBufferDeviceAddress.deviceAddress =
        getBufferDeviceAddress(scene->vertices.buffer);

    indexBufferDeviceAddress.deviceAddress =
        getBufferDeviceAddress(scene->indices.buffer);

    uint32_t numTriangles = static_cast<uint32_t>(scene->indices.count) / 3;
    uint32_t maxVertex = scene->vertices.count;

    // Build
    VkAccelerationStructureGeometryKHR accelerationStructureGeometry =
        create_info::accelerationStructureGeometryKHR();
    accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    accelerationStructureGeometry.geometry.triangles.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    accelerationStructureGeometry.geometry.triangles.vertexFormat =
        VK_FORMAT_R32G32B32_SFLOAT;
    accelerationStructureGeometry.geometry.triangles.vertexData =
        vertexBufferDeviceAddress;
    accelerationStructureGeometry.geometry.triangles.maxVertex = maxVertex;
    accelerationStructureGeometry.geometry.triangles.vertexStride =
        sizeof(gltf_model::Vertex);
    accelerationStructureGeometry.geometry.triangles.indexType =
        VK_INDEX_TYPE_UINT32;
    accelerationStructureGeometry.geometry.triangles.indexData =
        indexBufferDeviceAddress;
    accelerationStructureGeometry.geometry.triangles.transformData
        .deviceAddress = 0;
    accelerationStructureGeometry.geometry.triangles.transformData.hostAddress =
        nullptr;

    // Get size info
    VkAccelerationStructureBuildGeometryInfoKHR
        accelerationStructureBuildGeometryInfo =
            create_info::accelerationStructureBuildGeometryInfoKHR();
    accelerationStructureBuildGeometryInfo.type =
        VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationStructureBuildGeometryInfo.flags =
        VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationStructureBuildGeometryInfo.geometryCount = 1;
    accelerationStructureBuildGeometryInfo.pGeometries =
        &accelerationStructureGeometry;

    VkAccelerationStructureBuildSizesInfoKHR
        accelerationStructureBuildSizesInfo =
            create_info::accelerationStructureBuildSizesInfoKHR();
    vkGetAccelerationStructureBuildSizesKHR(
        *m_deviceHandler, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &accelerationStructureBuildGeometryInfo, &numTriangles,
        &accelerationStructureBuildSizesInfo);

    createAccelerationStructure(bottomLevelAS,
                                VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
                                accelerationStructureBuildSizesInfo);

    // Create a small scratch buffer used during build of the bottom level
    // acceleration structure
    ScratchBuffer scratchBuffer = createScratchBuffer(
        accelerationStructureBuildSizesInfo.buildScratchSize);

    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo =
        create_info::accelerationStructureBuildGeometryInfoKHR();
    accelerationBuildGeometryInfo.type =
        VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationBuildGeometryInfo.flags =
        VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationBuildGeometryInfo.mode =
        VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    accelerationBuildGeometryInfo.dstAccelerationStructure =
        bottomLevelAS.handle;
    accelerationBuildGeometryInfo.geometryCount = 1;
    accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
    accelerationBuildGeometryInfo.scratchData.deviceAddress =
        scratchBuffer.deviceAddress;

    VkAccelerationStructureBuildRangeInfoKHR
        accelerationStructureBuildRangeInfo{};
    accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
    accelerationStructureBuildRangeInfo.primitiveOffset = 0;
    accelerationStructureBuildRangeInfo.firstVertex = 0;
    accelerationStructureBuildRangeInfo.transformOffset = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR *>
        accelerationBuildStructureRangeInfos = {
            &accelerationStructureBuildRangeInfo};

    // Build the acceleration structure on the device via a one-time command
    // buffer submission Some implementations may support acceleration structure
    // building on the host
    // (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands),
    // but we prefer device builds
    VkCommandBuffer commandBuffer = m_commandBuffer->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    vkCmdBuildAccelerationStructuresKHR(
        commandBuffer, 1, &accelerationBuildGeometryInfo,
        accelerationBuildStructureRangeInfos.data());
    m_commandBuffer->flushCommandBuffer(commandBuffer,
                                        m_deviceHandler->graphicsQueue);

    deleteScratchBuffer(scratchBuffer);
}

/*
    The top level acceleration structure contains the scene's object instances
*/
void Raytracer::createTopLevelAccelerationStructure() {
    VkTransformMatrixKHR transformMatrix = {1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F,
                                            0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F};

    VkAccelerationStructureInstanceKHR instance{};
    instance.transform = transformMatrix;
    instance.instanceCustomIndex = 0;
    instance.mask = 0xFF;
    instance.instanceShaderBindingTableRecordOffset = 0;
    instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    instance.accelerationStructureReference = bottomLevelAS.deviceAddress;

    // Buffer for instance data
    Raytracer::Buffer instancesBuffer;
    VK_CHECK(m_deviceHandler->createBuffer(
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        sizeof(VkAccelerationStructureInstanceKHR), &instancesBuffer.buffer,
        &instancesBuffer.memory, &instance));

    VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
    instanceDataDeviceAddress.deviceAddress =
        getBufferDeviceAddress(instancesBuffer.buffer);

    VkAccelerationStructureGeometryKHR accelerationStructureGeometry =
        create_info::accelerationStructureGeometryKHR();
    accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    accelerationStructureGeometry.geometry.instances.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
    accelerationStructureGeometry.geometry.instances.data =
        instanceDataDeviceAddress;

    // Get size info
    VkAccelerationStructureBuildGeometryInfoKHR
        accelerationStructureBuildGeometryInfo =
            create_info::accelerationStructureBuildGeometryInfoKHR();
    accelerationStructureBuildGeometryInfo.type =
        VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    accelerationStructureBuildGeometryInfo.flags =
        VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationStructureBuildGeometryInfo.geometryCount = 1;
    accelerationStructureBuildGeometryInfo.pGeometries =
        &accelerationStructureGeometry;

    uint32_t primitive_count = 1;

    VkAccelerationStructureBuildSizesInfoKHR
        accelerationStructureBuildSizesInfo =
            create_info::accelerationStructureBuildSizesInfoKHR();
    vkGetAccelerationStructureBuildSizesKHR(
        *m_deviceHandler, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &accelerationStructureBuildGeometryInfo, &primitive_count,
        &accelerationStructureBuildSizesInfo);

    // @todo: as return value?
    createAccelerationStructure(topLevelAS,
                                VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
                                accelerationStructureBuildSizesInfo);

    // Create a small scratch buffer used during build of the top level
    // acceleration structure
    ScratchBuffer scratchBuffer = createScratchBuffer(
        accelerationStructureBuildSizesInfo.buildScratchSize);

    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo =
        create_info::accelerationStructureBuildGeometryInfoKHR();
    accelerationBuildGeometryInfo.type =
        VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    accelerationBuildGeometryInfo.flags =
        VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationBuildGeometryInfo.mode =
        VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    accelerationBuildGeometryInfo.dstAccelerationStructure = topLevelAS.handle;
    accelerationBuildGeometryInfo.geometryCount = 1;
    accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
    accelerationBuildGeometryInfo.scratchData.deviceAddress =
        scratchBuffer.deviceAddress;

    VkAccelerationStructureBuildRangeInfoKHR
        accelerationStructureBuildRangeInfo{};
    accelerationStructureBuildRangeInfo.primitiveCount = 1;
    accelerationStructureBuildRangeInfo.primitiveOffset = 0;
    accelerationStructureBuildRangeInfo.firstVertex = 0;
    accelerationStructureBuildRangeInfo.transformOffset = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR *>
        accelerationBuildStructureRangeInfos = {
            &accelerationStructureBuildRangeInfo};

    // Build the acceleration structure on the device via a one-time command
    // buffer submission Some implementations may support acceleration structure
    // building on the host
    // (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands),
    // but we prefer device builds
    VkCommandBuffer commandBuffer = m_commandBuffer->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    vkCmdBuildAccelerationStructuresKHR(
        commandBuffer, 1, &accelerationBuildGeometryInfo,
        accelerationBuildStructureRangeInfos.data());
    m_commandBuffer->flushCommandBuffer(commandBuffer,
                                        m_deviceHandler->graphicsQueue);

    deleteScratchBuffer(scratchBuffer);
    instancesBuffer.destroy(*m_deviceHandler);
}

/*
    Create the Shader Binding Tables that binds the programs and top-level
   acceleration structure

    SBT Layout used in this sample:

        /-----------\
        | raygen    |
        |-----------|
        | miss      |
        |-----------|
        | hit       |
        \-----------/

*/
void Raytracer::createShaderBindingTables() {
    const uint32_t handleSize =
        rayTracingPipelineProperties.shaderGroupHandleSize;
    const uint32_t handleSizeAligned = utils::alignedSize(
        rayTracingPipelineProperties.shaderGroupHandleSize,
        rayTracingPipelineProperties.shaderGroupHandleAlignment);
    const auto groupCount = static_cast<uint32_t>(shaderGroups.size());
    const uint32_t sbtSize = groupCount * handleSizeAligned;

    std::vector<uint8_t> shaderHandleStorage(sbtSize);
    VK_CHECK(vkGetRayTracingShaderGroupHandlesKHR(*m_deviceHandler, pipeline, 0,
                                                  groupCount, sbtSize,
                                                  shaderHandleStorage.data()));

    createShaderBindingTable(shaderBindingTables.raygen, 1);
    // We are using two miss shaders
    createShaderBindingTable(shaderBindingTables.miss, 2);
    createShaderBindingTable(shaderBindingTables.hit, 1);

    // Copy handles
    memcpy(shaderBindingTables.raygen.mapped, shaderHandleStorage.data(),
           handleSize);
    // We are using two miss shaders, so we need to get two handles for the miss
    // shader binding table
    memcpy(shaderBindingTables.miss.mapped,
           shaderHandleStorage.data() + handleSizeAligned, handleSize * 2);
    memcpy(shaderBindingTables.hit.mapped,
           shaderHandleStorage.data() + handleSizeAligned * 3, handleSize);
}

/*
    Create the descriptor sets used for the ray tracing dispatch
*/
void Raytracer::createDescriptorSets() {
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
         static_cast<uint32_t>(scene->textures.size())},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount =
        static_cast<uint32_t>(poolSizes.size());
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    VK_CHECK(vkCreateDescriptorPool(*m_deviceHandler, &descriptorPoolCreateInfo,
                                    nullptr, &descriptorPool));

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
    descriptorSetAllocateInfo.descriptorSetCount = 1;

    VK_CHECK(vkAllocateDescriptorSets(
        *m_deviceHandler, &descriptorSetAllocateInfo, &descriptorSet));

    VkWriteDescriptorSetAccelerationStructureKHR
        descriptorAccelerationStructureInfo{};
    descriptorAccelerationStructureInfo.sType =
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
    descriptorAccelerationStructureInfo.pAccelerationStructures =
        &topLevelAS.handle;

    VkWriteDescriptorSet accelerationStructureWrite{};
    accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // The specialized acceleration structure descriptor has to be chained
    accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
    accelerationStructureWrite.dstSet = descriptorSet;
    accelerationStructureWrite.dstBinding = 0;
    accelerationStructureWrite.descriptorCount = 1;
    accelerationStructureWrite.descriptorType =
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

    VkDescriptorImageInfo storageImageDescriptor{
        VK_NULL_HANDLE, storageImage.view, VK_IMAGE_LAYOUT_GENERAL};
    VkDescriptorBufferInfo vertexBufferDescriptor{scene->vertices.buffer, 0,
                                                  VK_WHOLE_SIZE};
    VkDescriptorBufferInfo indexBufferDescriptor{scene->indices.buffer, 0,
                                                 VK_WHOLE_SIZE};
    VkDescriptorBufferInfo lightsBufferDescriptor{this->lights.buffer, 0,
                                                  VK_WHOLE_SIZE};
    VkDescriptorBufferInfo colorBufferDescriptorInfo{this->colorBuffer.buffer,
                                                     0, VK_WHOLE_SIZE};

    VkSamplerCreateInfo createInfo =
        create_info::samplerCreateInfo(VK_FILTER_LINEAR);

    VK_CHECK(vkCreateSampler(*m_deviceHandler, &createInfo, nullptr, &sampler));
    textureDescriptors.clear();

    for (auto &texture : scene->textures) {
        textureDescriptors.push_back(texture.descriptor);
    }

    VkDescriptorImageInfo samplerInfo = {};
    samplerInfo.sampler = sampler;

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        // Binding 0: Top level acceleration structure
        accelerationStructureWrite,
        // Binding 1: Ray tracing result image
        create_info::writeDescriptorSet(descriptorSet,
                                        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1,
                                        &storageImageDescriptor),
        // Binding 2: Uniform data
        create_info::writeDescriptorSet(descriptorSet,
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2,
                                        &ubo.descriptor),
        // Binding 4: Scene index buffer
        create_info::writeDescriptorSet(descriptorSet,
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3,
                                        &lightsBufferDescriptor),
        // Binding 3: Scene vertex buffer
        create_info::writeDescriptorSet(descriptorSet,
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4,
                                        &vertexBufferDescriptor),
        // Binding 4: Scene index buffer
        create_info::writeDescriptorSet(descriptorSet,
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5,
                                        &indexBufferDescriptor),
        // Binding 5: Texture sampler
        create_info::writeDescriptorSet(
            descriptorSet, VK_DESCRIPTOR_TYPE_SAMPLER, 6, &samplerInfo),

        // // Binding 6: Sampled textures
        create_info::writeDescriptorSet(
            descriptorSet, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 7,
            textureDescriptors.data(), textureDescriptors.size()),

        // Binding 8: Scene color buffer
        create_info::writeDescriptorSet(descriptorSet,
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8,
                                        &colorBufferDescriptorInfo),
    };

    vkUpdateDescriptorSets(*m_deviceHandler,
                           static_cast<uint32_t>(writeDescriptorSets.size()),
                           writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
}

/*
    Create our ray tracing pipeline
*/
void Raytracer::createRayTracingPipeline() {
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0: Acceleration structure
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
            VK_SHADER_STAGE_RAYGEN_BIT_KHR |
                VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
            0),
        // Binding 1: Storage image
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR,
            1),
        // Binding 2: Uniform buffer
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_RAYGEN_BIT_KHR |
                VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                VK_SHADER_STAGE_MISS_BIT_KHR,
            2),
        // Binding 3: Lights buffer
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 3),
        // Binding 4: Vertex buffer
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 4),
        // Binding 5: Index buffer
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 5),
        // Binding 6: Uniform buffer
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 6),
        // // Binding 7: Uniform buffer
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 7, scene->textures.size()),
        // // Binding 8: Color buffer
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR,
            8),
    };

    std::vector<VkDescriptorBindingFlags> flags(
        setLayoutBindings.size(),
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
    bindingFlags.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlags.pNext = nullptr;
    bindingFlags.pBindingFlags = flags.data();
    bindingFlags.bindingCount = flags.size();

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI =
        create_info::descriptorSetLayoutCreateInfo(setLayoutBindings);
    descriptorSetLayoutCI.flags =
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    descriptorSetLayoutCI.pNext = &bindingFlags;

    VK_CHECK(vkCreateDescriptorSetLayout(*m_deviceHandler,
                                         &descriptorSetLayoutCI, nullptr,
                                         &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCI =
        create_info::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
    VK_CHECK(vkCreatePipelineLayout(*m_deviceHandler, &pPipelineLayoutCI,
                                    nullptr, &pipelineLayout));

    /*
        Setup ray tracing shader groups
    */
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    // Ray generation group
    {
        shaderStages.push_back(loadShader("shaders/raygen.rgen.spv",
                                          VK_SHADER_STAGE_RAYGEN_BIT_KHR));
        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
        shaderGroup.sType =
            VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroup.generalShader =
            static_cast<uint32_t>(shaderStages.size()) - 1;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);
    }

    // Miss group
    {
        shaderStages.push_back(
            loadShader("shaders/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));
        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
        shaderGroup.sType =
            VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroup.generalShader =
            static_cast<uint32_t>(shaderStages.size()) - 1;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);
        // Second shader for shadows
        shaderStages.push_back(loadShader("shaders/shadow.rmiss.spv",
                                          VK_SHADER_STAGE_MISS_BIT_KHR));
        shaderGroup.generalShader =
            static_cast<uint32_t>(shaderStages.size()) - 1;
        shaderGroups.push_back(shaderGroup);
    }

    // Closest hit group
    {
        shaderStages.push_back(loadShader("shaders/closesthit.rchit.spv",
                                          VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
        shaderGroup.sType =
            VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type =
            VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.closestHitShader =
            static_cast<uint32_t>(shaderStages.size()) - 1;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);
    }

    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
    rayTracingPipelineCI.sType =
        VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rayTracingPipelineCI.stageCount =
        static_cast<uint32_t>(shaderStages.size());
    rayTracingPipelineCI.pStages = shaderStages.data();
    rayTracingPipelineCI.groupCount =
        static_cast<uint32_t>(shaderGroups.size());
    rayTracingPipelineCI.pGroups = shaderGroups.data();
    rayTracingPipelineCI.maxPipelineRayRecursionDepth = 4;
    rayTracingPipelineCI.layout = pipelineLayout;
    VK_CHECK(vkCreateRayTracingPipelinesKHR(
        *m_deviceHandler, VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
        &rayTracingPipelineCI, nullptr, &pipeline));
}

VkResult Raytracer::Buffer::map(VkDevice device, VkDeviceSize size,
                                VkDeviceSize offset) {
    return vkMapMemory(device, memory, offset, size, 0, &mapped);
}

void Raytracer::setupLightsBuffer() {
    if (lights.lights.empty()) {
        return;
    }

    lights.size = lights.lights.size() * sizeof(glm::vec4);

    lights.usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    lights.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uniformData.lightsCount = lights.lights.size();

    VK_CHECK(m_deviceHandler->createBuffer(
        lights.usageFlags, lights.memoryPropertyFlags, lights.size,
        &lights.buffer, &lights.memory, lights.mapped));

    if (descriptorSet == VK_NULL_HANDLE) {
        return;
    }

    vkMapMemory(*m_deviceHandler, lights.memory, 0, lights.size, 0,
                &lights.mapped);
    memcpy(&lights.mapped, lights.lights.data(), (size_t)lights.size);

    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = this->lights.memory;
    mappedRange.offset = 0;
    mappedRange.size = this->lights.size;
    VK_CHECK(
        vkFlushMappedMemoryRanges(*this->m_deviceHandler, 1, &mappedRange));

    updateUniformBuffers();

    VkDescriptorBufferInfo lightsBufferDescriptor{this->lights.buffer, 0,
                                                  VK_WHOLE_SIZE};

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        // Binding 3: Scene index buffer
        create_info::writeDescriptorSet(this->descriptorSet,
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3,
                                        &lightsBufferDescriptor),
    };

    vkUpdateDescriptorSets(*m_deviceHandler,
                           static_cast<uint32_t>(writeDescriptorSets.size()),
                           writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
}

void Raytracer::setupColorsBuffer(bool setupDescr) {
    colorBuffer.size = m_swapChain->swapChainExtent.width *
                       m_swapChain->swapChainExtent.height * sizeof(glm::vec4) *
                       2;

    colorBuffer.usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    colorBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VK_CHECK(m_deviceHandler->createBuffer(
        colorBuffer.usageFlags, colorBuffer.memoryPropertyFlags,
        colorBuffer.size, &colorBuffer.buffer, &colorBuffer.memory, nullptr));

    if (!setupDescr) {
        return;
    }

    VkDescriptorBufferInfo colorBufferDescriptorInfo{this->colorBuffer.buffer,
                                                     0, VK_WHOLE_SIZE};

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        // Binding 8: Scene color buffer
        create_info::writeDescriptorSet(this->descriptorSet,
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8,
                                        &colorBufferDescriptorInfo),
    };

    vkUpdateDescriptorSets(*m_deviceHandler,
                           static_cast<uint32_t>(writeDescriptorSets.size()),
                           writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
}

void Raytracer::cleanupColorsBuffer() {
    if (colorBuffer.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(*m_deviceHandler, colorBuffer.buffer, nullptr);
    }

    if (colorBuffer.memory != VK_NULL_HANDLE) {
        vkUnmapMemory(*m_deviceHandler, colorBuffer.memory);
    }

    if (colorBuffer.buffer != VK_NULL_HANDLE) {
        vkFreeMemory(*m_deviceHandler, colorBuffer.memory, nullptr);
    }
}

void Raytracer::updateLightsBuffer(std::vector<glm::vec4> newLights) {
    cleanupLightsBuffer();
    this->lights.lights = std::move(newLights);
    setupLightsBuffer();
}

void Raytracer::cleanupLightsBuffer() {
    if (lights.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(*m_deviceHandler, lights.buffer, nullptr);
    }

    if (lights.memory != VK_NULL_HANDLE) {
        vkUnmapMemory(*m_deviceHandler, lights.memory);
    }

    if (lights.buffer != VK_NULL_HANDLE) {
        vkFreeMemory(*m_deviceHandler, lights.memory, nullptr);
    }
}

/*
    Create the uniform buffer used to pass matrices to the ray tracing ray
   generation shader
*/
void Raytracer::createUniformBuffer() {
    VK_CHECK(m_deviceHandler->createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        sizeof(UniformData), &ubo.buffer, &ubo.memory, nullptr));
    VK_CHECK(ubo.map(*m_deviceHandler));

    ubo.descriptor.offset = 0;
    ubo.descriptor.buffer = ubo.buffer;
    ubo.descriptor.range = sizeof(UniformData);

    updateUniformBuffers();
}

void Raytracer::updateUniformBuffers() {
    uniformData.projInverse = glm::inverse(glm::identity<glm::mat4>());
    uniformData.viewInverse = glm::inverse(glm::identity<glm::mat4>());
    uniformData.lightsCount = lights.lights.size();
    // Pass the vertex size to the shader for unpacking vertices
    uniformData.vertexSize = sizeof(gltf_model::Vertex);
    memcpy(ubo.mapped, &uniformData, sizeof(uniformData));
}

void Raytracer::updateUniformBuffers(glm::mat4 proj, glm::mat4 view) {
    glm::mat4 invYAxisMatrix = {
        1.0F, 0.0F, 0.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F,  0.0F, 1.0F,
    };
    uniformData.projInverse = glm::inverse(proj);
    uniformData.viewInverse = glm::inverse(view * invYAxisMatrix);
    uniformData.width = m_swapChain->swapChainExtent.width;
    uniformData.lightsCount = lights.lights.size();
    uniformData.vertexSize = sizeof(gltf_model::Vertex);
    memcpy(ubo.mapped, &uniformData, sizeof(uniformData));
}

void Raytracer::handleResize() {
    vkDeviceWaitIdle(*m_deviceHandler);

    m_swapChain->cleanup();
    m_swapChain->init();
    updateRenderPass();

    vkDeviceWaitIdle(*m_deviceHandler);

    cleanupColorsBuffer();
    setupColorsBuffer();

    uint32_t width = this->m_swapChain->swapChainExtent.width;
    uint32_t height = this->m_swapChain->swapChainExtent.height;
    createStorageImage(this->m_swapChain->swapChainImageFormat,
                       {width, height, 1});

    VkDescriptorImageInfo storageImageDescriptor{
        VK_NULL_HANDLE, storageImage.view, VK_IMAGE_LAYOUT_GENERAL};

    VkWriteDescriptorSet resultImageWrite = create_info::writeDescriptorSet(
        descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1,
        &storageImageDescriptor);

    vkUpdateDescriptorSets(*m_deviceHandler, 1, &resultImageWrite, 0,
                           VK_NULL_HANDLE);

    clearCommandBuffers();
    makeCommandBuffers();
    buildCommandBuffers();
    updateUniformBuffers();
}

void Raytracer::makeCommandBuffers() {
    drawCmdBuffers.resize(m_swapChain->swapChainFramebuffers.size());
    VkCommandBufferAllocateInfo allocInfo = create_info::commandBufferAllocInfo(
        m_commandBuffer->commandPool, drawCmdBuffers.size());
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(m_deviceHandler->logicalDevice,
                                      &allocInfo, drawCmdBuffers.data()))
}

void Raytracer::clearCommandBuffers() {
    vkFreeCommandBuffers(
        m_deviceHandler->logicalDevice, m_commandBuffer->commandPool,
        static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
    drawCmdBuffers.clear();
}

void Raytracer::buildCommandBuffers() {
    uint32_t width = m_swapChain->swapChainExtent.width;
    uint32_t height = m_swapChain->swapChainExtent.height;

    VkCommandBufferBeginInfo cmdBufInfo = create_info::commandBufferBeginInfo();

    VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
                                                0, 1};

    for (size_t i = 0; i < drawCmdBuffers.size(); ++i) {
        VK_CHECK(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

        /*
            Dispatch the ray tracing commands
        */
        vkCmdBindPipeline(drawCmdBuffers[i],
                          VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);

        vkCmdBindDescriptorSets(
            drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
            pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        VkStridedDeviceAddressRegionKHR emptySbtEntry = {};
        vkCmdTraceRaysKHR(
            drawCmdBuffers[i],
            &shaderBindingTables.raygen.stridedDeviceAddressRegion,
            &shaderBindingTables.miss.stridedDeviceAddressRegion,
            &shaderBindingTables.hit.stridedDeviceAddressRegion, &emptySbtEntry,
            width, height, 1);

        /*
            Copy ray tracing output to swap chain image
        */

        // Prepare current swap chain image as transfer destination
        utils::setImageLayout(
            drawCmdBuffers[i], m_swapChain->swapChainImages[i],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            subresourceRange);

        // Prepare ray tracing output image as transfer source
        utils::setImageLayout(
            drawCmdBuffers[i], storageImage.image, VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

        VkImageCopy copyRegion{};
        copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copyRegion.srcOffset = {0, 0, 0};
        copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copyRegion.dstOffset = {0, 0, 0};
        copyRegion.extent = {width, height, 1};
        vkCmdCopyImage(drawCmdBuffers[i], storageImage.image,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       m_swapChain->swapChainImages[i],
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        // Transition swap chain image back for presentation
        utils::setImageLayout(
            drawCmdBuffers[i], m_swapChain->swapChainImages[i],
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, subresourceRange);

        // Transition ray tracing output image back to general layout
        utils::setImageLayout(drawCmdBuffers[i], storageImage.image,
                              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                              VK_IMAGE_LAYOUT_GENERAL, subresourceRange);

        // drawUI(drawCmdBuffers[i], m_swapChain->swapChainFramebuffers[i]);

        VK_CHECK(vkEndCommandBuffer(drawCmdBuffers[i]));
    }
}

void Raytracer::prepareFrame() {
    vkWaitForFences(m_deviceHandler->logicalDevice, 1,
                    &m_swapChain->inFlightFences[curFrame], VK_TRUE,
                    UINT64_MAX);

    VkResult result = m_swapChain->getNextImage(curFrame, &imageIdx);

    // Recreate the swapchain if it's no longer compatible with the surface
    // (OUT_OF_DATE) SRS - If no longer optimal (VK_SUBOPTIMAL_KHR), wait
    // until submitFrame() in case number of swapchain images will change on
    // resize
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            handleResize();
        }
        return;
    }

    VK_CHECK(result);
}

void Raytracer::submitFrame() {

    VkResult result =
        m_swapChain->queuePresent(m_deviceHandler->presentQueue, curFrame);
    // Recreate the swapchain if it's no longer compatible with the surface
    // (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
        handleResize();
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return;
        }
    }

    VK_CHECK(result);
    VK_CHECK(vkQueueWaitIdle(m_deviceHandler->presentQueue));
}

void Raytracer::renderFrame() {
    prepareFrame();

    vkResetFences(m_deviceHandler->logicalDevice, 1,
                  &m_swapChain->inFlightFences[curFrame]);

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[curFrame];

    submitInfo.pWaitSemaphores =
        &this->m_swapChain->imageAvailableSemaphores[curFrame];

    submitInfo.pSignalSemaphores =
        &this->m_swapChain->renderFinishedSemaphores[curFrame];

    VK_CHECK(vkQueueSubmit(m_deviceHandler->graphicsQueue, 1, &submitInfo,
                           m_swapChain->inFlightFences[curFrame]));

    submitFrame();

    curFrame = (curFrame + 1) % m_swapChain->swapChainFramebuffers.size();
}

void Raytracer::Buffer::destroy(VkDevice device) const {
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, buffer, nullptr);
    }
    if (memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, memory, nullptr);
    }
}
