/*
 * Vulkan glTF model and texture loading class based on tinyglTF
 * (https://github.com/syoyo/tinygltf)
 *
 * Copyright (C) 2018 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT)
 * (http://opensource.org/licenses/MIT)
 */

#include "gltf_model/model.hpp"
#include "common.hpp"
#include "gltf_model/gltf_common.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/staging_buffer.hpp"
#include "vulkan_utils/utils.hpp"

/*
    We use a custom image loading function with tinyglTF, so we can do custom
   stuff loading ktx textures
*/
bool loadImageDataFunc(tinygltf::Image *image, const int imageIndex,
                       std::string *error, std::string *warning, int req_width,
                       int req_height, const unsigned char *bytes, int size,
                       void *userData) {
    // KTX files will be handled by our own code
    if (image->uri.find_last_of(".") != std::string::npos) {
        if (image->uri.substr(image->uri.find_last_of(".") + 1) == "ktx") {
            return true;
        }
    }

    return tinygltf::LoadImageData(image, imageIndex, error, warning, req_width,
                                   req_height, bytes, size, userData);
}

bool loadImageDataFuncEmpty(tinygltf::Image *image, const int imageIndex,
                            std::string *error, std::string *warning,
                            int req_width, int req_height,
                            const unsigned char *bytes, int size,
                            void *userData) {
    // This function will be used for samples that don't require images to be
    // loaded
    UNUSED(image);
    UNUSED(imageIndex);
    UNUSED(error);
    UNUSED(warning);
    UNUSED(req_width);
    UNUSED(req_height);
    UNUSED(bytes);
    UNUSED(size);
    UNUSED(userData);

    return true;
}

gltf_model::Texture *gltf_model::Model::getTexture(uint32_t index) {

    if (index < textures.size()) {
        return &textures[index];
    }
    return nullptr;
}

void gltf_model::Model::createEmptyTexture(VkQueue transferQueue) {
    emptyTexture.deviceHandler = m_deviceHandler;
    emptyTexture.commandBuffer = m_commandBuffer;
    emptyTexture.width = 1;
    emptyTexture.height = 1;
    emptyTexture.layerCount = 1;
    emptyTexture.mipLevels = 1;

    size_t bufferSize = emptyTexture.width * emptyTexture.height * 4;
    auto *buffer = new unsigned char[bufferSize];
    memset(buffer, 0, bufferSize);

    buffer::StagingBuffer buf =
        buffer::StagingBuffer(m_deviceHandler, m_commandBuffer, bufferSize);
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(*m_deviceHandler, buf.buffer, &memReqs);
    buf.copy(buffer, bufferSize);

    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = emptyTexture.width;
    bufferCopyRegion.imageExtent.height = emptyTexture.height;
    bufferCopyRegion.imageExtent.depth = 1;

    // Create optimal tiled target image
    VkImageCreateInfo imageCreateInfo = create_info::imageCreateInfo(
        VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent = {emptyTexture.width, emptyTexture.height, 1};

    VK_CHECK(vkCreateImage(*m_deviceHandler, &imageCreateInfo, nullptr,
                           &emptyTexture.image));

    VkMemoryAllocateInfo memAllocInfo = create_info::memoryAllocInfo(
        memReqs.size,
        m_deviceHandler->getMemoryType(memReqs.memoryTypeBits,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

    vkGetImageMemoryRequirements(*m_deviceHandler, emptyTexture.image,
                                 &memReqs);

    VK_CHECK(vkAllocateMemory(*m_deviceHandler, &memAllocInfo, nullptr,
                              &emptyTexture.deviceMemory));
    VK_CHECK(vkBindImageMemory(*m_deviceHandler, emptyTexture.image,
                               emptyTexture.deviceMemory, 0));

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    VkCommandBuffer copyCmd = m_commandBuffer->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    utils::setImageLayout(
        copyCmd, emptyTexture.image, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

    vkCmdCopyBufferToImage(copyCmd, buf.buffer, emptyTexture.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &bufferCopyRegion);
    utils::setImageLayout(
        copyCmd, emptyTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

    m_commandBuffer->flushCommandBuffer(copyCmd, transferQueue);

    emptyTexture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Clean up staging resources
    VkSamplerCreateInfo samplerCreateInfo =
        create_info::samplerCreateInfo(VK_FILTER_LINEAR);
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.maxAnisotropy = 1.0F;
    VK_CHECK(vkCreateSampler(*m_deviceHandler, &samplerCreateInfo, nullptr,
                             &emptyTexture.sampler));

    VkImageViewCreateInfo viewCreateInfo = create_info::imageViewCreateInfo(
        emptyTexture.image, VK_FORMAT_R8G8B8A8_UNORM);
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    viewCreateInfo.subresourceRange.levelCount = 1;
    VK_CHECK(vkCreateImageView(*m_deviceHandler, &viewCreateInfo, nullptr,
                               &emptyTexture.view));

    emptyTexture.descriptor.imageLayout =
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    emptyTexture.descriptor.imageView = emptyTexture.view;
    emptyTexture.descriptor.sampler = emptyTexture.sampler;
}

/*
    glTF model loading and rendering class
*/
gltf_model::Model::~Model() {
    vkDestroyBuffer(*m_deviceHandler, vertices.buffer, nullptr);
    vkFreeMemory(*m_deviceHandler, vertices.memory, nullptr);
    vkDestroyBuffer(*m_deviceHandler, indices.buffer, nullptr);
    vkFreeMemory(*m_deviceHandler, indices.memory, nullptr);
    for (auto texture : textures) {
        texture.destroy();
    }
    for (auto *node : nodes) {
        delete node;
    }
    for (auto *skin : skins) {
        delete skin;
    }
    if (descriptorSetLayoutUbo != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(*m_deviceHandler, descriptorSetLayoutUbo,
                                     nullptr);
        descriptorSetLayoutUbo = VK_NULL_HANDLE;
    }
    if (descriptorSetLayoutImage != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(*m_deviceHandler, descriptorSetLayoutImage,
                                     nullptr);
        descriptorSetLayoutImage = VK_NULL_HANDLE;
    }
    vkDestroyDescriptorPool(*m_deviceHandler, descriptorPool, nullptr);
    emptyTexture.destroy();
}

void gltf_model::Model::loadNode(gltf_model::Node *parent,
                                 const tinygltf::Node &node, uint32_t nodeIndex,
                                 const tinygltf::Model &model,
                                 std::vector<uint32_t> &indexBuffer,
                                 std::vector<Vertex> &vertexBuffer,
                                 float globalscale) {
    auto *newNode = new Node{};
    newNode->index = nodeIndex;
    newNode->parent = parent;
    newNode->name = node.name;
    newNode->skinIndex = node.skin;
    newNode->matrix = glm::mat4(1.0F);

    // Generate local node matrix
    auto translation = glm::vec3(0.0F);
    if (node.translation.size() == 3) {
        translation = glm::make_vec3(node.translation.data());
        newNode->translation = translation;
    }
    if (node.rotation.size() == 4) {
        glm::quat quad = glm::make_quat(node.rotation.data());
        newNode->rotation = glm::mat4(quad);
    }
    auto scale = glm::vec3(1.0F);
    if (node.scale.size() == 3) {
        scale = glm::make_vec3(node.scale.data());
        newNode->scale = scale;
    }
    if (node.matrix.size() == 16) {
        newNode->matrix = glm::make_mat4x4(node.matrix.data());
        if (globalscale != 1.0F) {
            // newNode->matrix = glm::scale(newNode->matrix,
            // glm::vec3(globalscale));
        }
    };

    // Node with children
    if (!node.children.empty()) {
        for (const auto &child : node.children) {
            loadNode(newNode, model.nodes[child], child, model, indexBuffer,
                     vertexBuffer, globalscale);
        }
    }

    // Node contains mesh data
    if (node.mesh > -1) {
        const tinygltf::Mesh mesh = model.meshes[node.mesh];
        Mesh *newMesh =
            new Mesh(m_deviceHandler, m_commandBuffer, newNode->matrix);
        newMesh->name = mesh.name;

        for (const auto &prim : mesh.primitives) {
            const tinygltf::Primitive &primitive = prim;
            if (primitive.indices < 0) {
                continue;
            }
            auto indexStart = static_cast<uint32_t>(indexBuffer.size());
            auto vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            glm::vec3 posMin{};
            glm::vec3 posMax{};
            bool hasSkin = false;
            // Vertices
            {
                const float *bufferPos = nullptr;
                const float *bufferNormals = nullptr;
                const float *bufferTexCoords = nullptr;
                const float *bufferColors = nullptr;
                const float *bufferTangents = nullptr;
                uint32_t numColorComponents = 0;
                const uint16_t *bufferJoints = nullptr;
                const float *bufferWeights = nullptr;

                // Position attribute is required
                assert(primitive.attributes.find("POSITION") !=
                       primitive.attributes.end());

                const tinygltf::Accessor &posAccessor =
                    model.accessors[primitive.attributes.find("POSITION")
                                        ->second];
                const tinygltf::BufferView &posView =
                    model.bufferViews[posAccessor.bufferView];
                bufferPos = reinterpret_cast<const float *>(
                    &(model.buffers[posView.buffer]
                          .data[posAccessor.byteOffset + posView.byteOffset]));
                posMin = glm::vec3(posAccessor.minValues[0],
                                   posAccessor.minValues[1],
                                   posAccessor.minValues[2]);
                posMax = glm::vec3(posAccessor.maxValues[0],
                                   posAccessor.maxValues[1],
                                   posAccessor.maxValues[2]);

                if (primitive.attributes.find("NORMAL") !=
                    primitive.attributes.end()) {
                    const tinygltf::Accessor &normAccessor =
                        model.accessors[primitive.attributes.find("NORMAL")
                                            ->second];
                    const tinygltf::BufferView &normView =
                        model.bufferViews[normAccessor.bufferView];
                    bufferNormals = reinterpret_cast<const float *>(
                        &(model.buffers[normView.buffer]
                              .data[normAccessor.byteOffset +
                                    normView.byteOffset]));
                }

                if (primitive.attributes.find("TEXCOORD_0") !=
                    primitive.attributes.end()) {
                    const tinygltf::Accessor &uvAccessor =
                        model.accessors[primitive.attributes.find("TEXCOORD_0")
                                            ->second];
                    const tinygltf::BufferView &uvView =
                        model.bufferViews[uvAccessor.bufferView];
                    bufferTexCoords = reinterpret_cast<const float *>(&(
                        model.buffers[uvView.buffer]
                            .data[uvAccessor.byteOffset + uvView.byteOffset]));
                }

                if (primitive.attributes.find("COLOR_0") !=
                    primitive.attributes.end()) {
                    const tinygltf::Accessor &colorAccessor =
                        model.accessors[primitive.attributes.find("COLOR_0")
                                            ->second];
                    const tinygltf::BufferView &colorView =
                        model.bufferViews[colorAccessor.bufferView];
                    // Color buffer are either of type vec3 or vec4
                    numColorComponents =
                        colorAccessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3
                            ? 3
                            : 4;
                    bufferColors = reinterpret_cast<const float *>(
                        &(model.buffers[colorView.buffer]
                              .data[colorAccessor.byteOffset +
                                    colorView.byteOffset]));
                }

                if (primitive.attributes.find("TANGENT") !=
                    primitive.attributes.end()) {
                    const tinygltf::Accessor &tangentAccessor =
                        model.accessors[primitive.attributes.find("TANGENT")
                                            ->second];
                    const tinygltf::BufferView &tangentView =
                        model.bufferViews[tangentAccessor.bufferView];
                    bufferTangents = reinterpret_cast<const float *>(
                        &(model.buffers[tangentView.buffer]
                              .data[tangentAccessor.byteOffset +
                                    tangentView.byteOffset]));
                }

                // Skinning
                // Joints
                if (primitive.attributes.find("JOINTS_0") !=
                    primitive.attributes.end()) {
                    const tinygltf::Accessor &jointAccessor =
                        model.accessors[primitive.attributes.find("JOINTS_0")
                                            ->second];
                    const tinygltf::BufferView &jointView =
                        model.bufferViews[jointAccessor.bufferView];
                    bufferJoints = reinterpret_cast<const uint16_t *>(
                        &(model.buffers[jointView.buffer]
                              .data[jointAccessor.byteOffset +
                                    jointView.byteOffset]));
                }

                if (primitive.attributes.find("WEIGHTS_0") !=
                    primitive.attributes.end()) {
                    const tinygltf::Accessor &uvAccessor =
                        model.accessors[primitive.attributes.find("WEIGHTS_0")
                                            ->second];
                    const tinygltf::BufferView &uvView =
                        model.bufferViews[uvAccessor.bufferView];
                    bufferWeights = reinterpret_cast<const float *>(&(
                        model.buffers[uvView.buffer]
                            .data[uvAccessor.byteOffset + uvView.byteOffset]));
                }

                hasSkin = static_cast<bool>(bufferJoints) &&
                          static_cast<bool>(bufferWeights);

                vertexCount = static_cast<uint32_t>(posAccessor.count);

                for (size_t idx = 0; idx < posAccessor.count; idx++) {
                    Vertex vert{};
                    vert.pos =
                        glm::vec4(glm::make_vec3(&bufferPos[idx * 3]), 1.0F);
                    vert.normal = glm::normalize(
                        glm::vec3(bufferNormals != nullptr
                                      ? glm::make_vec3(&bufferNormals[idx * 3])
                                      : glm::vec3(0.0F)));
                    vert.uv = bufferTexCoords != nullptr
                                  ? glm::make_vec2(&bufferTexCoords[idx * 2])
                                  : glm::vec3(0.0F);
                    if (bufferColors != nullptr) {
                        switch (numColorComponents) {
                        case 3:
                            vert.color = glm::vec4(
                                glm::make_vec3(&bufferColors[idx * 3]), 1.0F);
                            [[fallthrough]];
                        case 4:
                            vert.color = glm::make_vec4(&bufferColors[idx * 4]);
                        }
                    } else {
                        vert.color = glm::vec4(1.0F);
                    }
                    vert.tangent = bufferTangents != nullptr
                                       ? glm::vec4(glm::make_vec4(
                                             &bufferTangents[idx * 4]))
                                       : glm::vec4(0.0F);
                    vert.joint0 =
                        hasSkin
                            ? glm::vec4(glm::make_vec4(&bufferJoints[idx * 4]))
                            : glm::vec4(0.0F);
                    vert.weight0 = hasSkin
                                       ? glm::make_vec4(&bufferWeights[idx * 4])
                                       : glm::vec4(0.0F);
                    vertexBuffer.push_back(vert);
                }
            }
            // Indices
            {
                const tinygltf::Accessor &accessor =
                    model.accessors[primitive.indices];
                const tinygltf::BufferView &bufferView =
                    model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer =
                    model.buffers[bufferView.buffer];

                indexCount = static_cast<uint32_t>(accessor.count);

                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    auto *buf = new uint32_t[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        bufferView.byteOffset],
                           accessor.count * sizeof(uint32_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    delete[] buf;
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    auto *buf = new uint16_t[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        bufferView.byteOffset],
                           accessor.count * sizeof(uint16_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    delete[] buf;
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    auto *buf = new uint8_t[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        bufferView.byteOffset],
                           accessor.count * sizeof(uint8_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    delete[] buf;
                    break;
                }
                default:
                    std::cerr << "Index component type "
                              << accessor.componentType << " not supported!"
                              << std::endl;
                    return;
                }
            }
            auto *newPrimitive = new Primitive(
                indexStart, indexCount,
                primitive.material > -1 ? materials[primitive.material]
                                        : materials.back());
            newPrimitive->firstVertex = vertexStart;
            newPrimitive->vertexCount = vertexCount;
            newPrimitive->setDimensions(posMin, posMax);
            newMesh->primitives.push_back(newPrimitive);
        }
        newNode->mesh = newMesh;
    }
    if (parent != nullptr) {
        parent->children.push_back(newNode);
    } else {
        nodes.push_back(newNode);
    }
    linearNodes.push_back(newNode);
}

void gltf_model::Model::loadSkins(tinygltf::Model &gltfModel) {
    for (tinygltf::Skin &source : gltfModel.skins) {
        Skin *newSkin = new Skin{};
        newSkin->name = source.name;

        // Find skeleton root node
        if (source.skeleton > -1) {
            newSkin->skeletonRoot = nodeFromIndex(source.skeleton);
        }

        // Find joint nodes
        for (int jointIndex : source.joints) {
            Node *node = nodeFromIndex(jointIndex);
            if (node != nullptr) {
                newSkin->joints.push_back(nodeFromIndex(jointIndex));
            }
        }

        // Get inverse bind matrices from buffer
        if (source.inverseBindMatrices > -1) {
            const tinygltf::Accessor &accessor =
                gltfModel.accessors[source.inverseBindMatrices];
            const tinygltf::BufferView &bufferView =
                gltfModel.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer =
                gltfModel.buffers[bufferView.buffer];
            newSkin->inverseBindMatrices.resize(accessor.count);
            memcpy(newSkin->inverseBindMatrices.data(),
                   &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                   accessor.count * sizeof(glm::mat4));
        }

        skins.push_back(newSkin);
    }
}

void gltf_model::Model::loadImages(
    tinygltf::Model &gltfModel, std::shared_ptr<device::DeviceHandler> device,
    std::shared_ptr<command_buffer::CommandBufferHandler> cmdBuf,
    VkQueue transferQueue) {

    for (tinygltf::Image &image : gltfModel.images) {
        gltf_model::Texture texture;
        texture.fromglTfImage(image, path, std::move(device), std::move(cmdBuf),
                              transferQueue);
        textures.push_back(texture);
    }
    // Create an empty texture to be used for empty material images
    createEmptyTexture(transferQueue);
}

void gltf_model::Model::loadMaterials(tinygltf::Model &gltfModel) {
    for (tinygltf::Material &mat : gltfModel.materials) {
        gltf_model::Material material(m_deviceHandler, m_commandBuffer);
        if (mat.values.find("baseColorTexture") != mat.values.end()) {
            material.baseColorTexture = getTexture(
                gltfModel
                    .textures[mat.values["baseColorTexture"].TextureIndex()]
                    .source);
        }
        // Metallic roughness workflow
        if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
            material.metallicRoughnessTexture =
                getTexture(gltfModel
                               .textures[mat.values["metallicRoughnessTexture"]
                                             .TextureIndex()]
                               .source);
        }
        if (mat.values.find("roughnessFactor") != mat.values.end()) {
            material.roughnessFactor =
                static_cast<float>(mat.values["roughnessFactor"].Factor());
        }
        if (mat.values.find("metallicFactor") != mat.values.end()) {
            material.metallicFactor =
                static_cast<float>(mat.values["metallicFactor"].Factor());
        }
        if (mat.values.find("baseColorFactor") != mat.values.end()) {
            material.baseColorFactor = glm::make_vec4(
                mat.values["baseColorFactor"].ColorFactor().data());
        }
        if (mat.additionalValues.find("normalTexture") !=
            mat.additionalValues.end()) {
            material.normalTexture =
                getTexture(gltfModel
                               .textures[mat.additionalValues["normalTexture"]
                                             .TextureIndex()]
                               .source);
        } else {
            material.normalTexture = &emptyTexture;
        }
        if (mat.additionalValues.find("emissiveTexture") !=
            mat.additionalValues.end()) {
            material.emissiveTexture =
                getTexture(gltfModel
                               .textures[mat.additionalValues["emissiveTexture"]
                                             .TextureIndex()]
                               .source);
        }
        if (mat.additionalValues.find("occlusionTexture") !=
            mat.additionalValues.end()) {
            material.occlusionTexture = getTexture(
                gltfModel
                    .textures[mat.additionalValues["occlusionTexture"]
                                  .TextureIndex()]
                    .source);
        }
        if (mat.additionalValues.find("alphaMode") !=
            mat.additionalValues.end()) {
            tinygltf::Parameter param = mat.additionalValues["alphaMode"];
            if (param.string_value == "BLEND") {
                material.alphaMode = Material::ALPHAMODE_BLEND;
            }
            if (param.string_value == "MASK") {
                material.alphaMode = Material::ALPHAMODE_MASK;
            }
        }
        if (mat.additionalValues.find("alphaCutoff") !=
            mat.additionalValues.end()) {
            material.alphaCutoff = static_cast<float>(
                mat.additionalValues["alphaCutoff"].Factor());
        }

        materials.push_back(material);
    }
    // Push a default material at the end of the list for meshes with no
    // material assigned
    materials.emplace_back(m_deviceHandler, m_commandBuffer);
}

void gltf_model::Model::loadAnimations(tinygltf::Model &gltfModel) {
    for (tinygltf::Animation &anim : gltfModel.animations) {
        gltf_model::Animation animation{};
        animation.name = anim.name;
        if (anim.name.empty()) {
            animation.name = std::to_string(animations.size());
        }

        // Samplers
        for (auto &samp : anim.samplers) {
            gltf_model::AnimationSampler sampler{};

            if (samp.interpolation == "LINEAR") {
                sampler.interpolation =
                    AnimationSampler::InterpolationType::LINEAR;
            }
            if (samp.interpolation == "STEP") {
                sampler.interpolation =
                    AnimationSampler::InterpolationType::STEP;
            }
            if (samp.interpolation == "CUBICSPLINE") {
                sampler.interpolation =
                    AnimationSampler::InterpolationType::CUBICSPLINE;
            }

            // Read sampler input time values
            {
                const tinygltf::Accessor &accessor =
                    gltfModel.accessors[samp.input];
                const tinygltf::BufferView &bufferView =
                    gltfModel.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer =
                    gltfModel.buffers[bufferView.buffer];

                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                auto *buf = new float[accessor.count];
                memcpy(
                    buf,
                    &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                    accessor.count * sizeof(float));
                for (size_t index = 0; index < accessor.count; index++) {
                    sampler.inputs.push_back(buf[index]);
                }
                delete[] buf;
                for (auto input : sampler.inputs) {
                    if (input < animation.start) {
                        animation.start = input;
                    };
                    if (input > animation.end) {
                        animation.end = input;
                    }
                }
            }

            // Read sampler output T/R/S values
            {
                const tinygltf::Accessor &accessor =
                    gltfModel.accessors[samp.output];
                const tinygltf::BufferView &bufferView =
                    gltfModel.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer =
                    gltfModel.buffers[bufferView.buffer];

                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                switch (accessor.type) {
                case TINYGLTF_TYPE_VEC3: {
                    auto *buf = new glm::vec3[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        bufferView.byteOffset],
                           accessor.count * sizeof(glm::vec3));
                    for (size_t index = 0; index < accessor.count; index++) {
                        sampler.outputsVec4.emplace_back(buf[index], 0.0F);
                    }
                    delete[] buf;
                    break;
                }
                case TINYGLTF_TYPE_VEC4: {
                    auto *buf = new glm::vec4[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        bufferView.byteOffset],
                           accessor.count * sizeof(glm::vec4));
                    for (size_t index = 0; index < accessor.count; index++) {
                        sampler.outputsVec4.push_back(buf[index]);
                    }
                    delete[] buf;
                    break;
                }
                default: {
                    std::cout << "unknown type" << std::endl;
                    break;
                }
                }
            }

            animation.samplers.push_back(sampler);
        }

        // Channels
        for (auto &source : anim.channels) {
            gltf_model::AnimationChannel channel{};

            if (source.target_path == "rotation") {
                channel.path = AnimationChannel::PathType::ROTATION;
            }
            if (source.target_path == "translation") {
                channel.path = AnimationChannel::PathType::TRANSLATION;
            }
            if (source.target_path == "scale") {
                channel.path = AnimationChannel::PathType::SCALE;
            }
            if (source.target_path == "weights") {
                std::cout << "weights not yet supported, skipping channel"
                          << std::endl;
                continue;
            }
            channel.samplerIndex = source.sampler;
            channel.node = nodeFromIndex(source.target_node);
            if (channel.node == nullptr) {
                continue;
            }

            animation.channels.push_back(channel);
        }

        animations.push_back(animation);
    }
}

void gltf_model::Model::loadFromFile(
    std::string filename, std::shared_ptr<device::DeviceHandler> device,
    std::shared_ptr<command_buffer::CommandBufferHandler> cmdBuf,
    VkQueue transferQueue, uint32_t fileLoadingFlags, float scale) {

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;

    if (static_cast<bool>(fileLoadingFlags &
                          FileLoadingFlags::DontLoadImages)) {
        gltfContext.SetImageLoader(loadImageDataFuncEmpty, nullptr);
    } else {
        gltfContext.SetImageLoader(loadImageDataFunc, nullptr);
    }

    size_t pos = filename.find_last_of('/');
    path = filename.substr(0, pos);

    std::string error;
    std::string warning;

    this->m_deviceHandler = std::move(device);
    this->m_commandBuffer = std::move(cmdBuf);

    bool fileLoaded =
        gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filename);

    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex> vertexBuffer;

    if (fileLoaded) {
        if (!static_cast<bool>(fileLoadingFlags &
                               FileLoadingFlags::DontLoadImages)) {
            loadImages(gltfModel, m_deviceHandler, m_commandBuffer,
                       transferQueue);
        }
        loadMaterials(gltfModel);
        const tinygltf::Scene &scene =
            gltfModel
                .scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene
                                                    : 0];
        // for (size_t i = 0; i < scene.nodes.size(); i++) {
        for (const auto &node_idx : scene.nodes) {
            const tinygltf::Node node = gltfModel.nodes[node_idx];
            loadNode(nullptr, node, node_idx, gltfModel, indexBuffer,
                     vertexBuffer, scale);
        }
        if (!gltfModel.animations.empty()) {
            loadAnimations(gltfModel);
        }
        loadSkins(gltfModel);

        for (auto *node : linearNodes) {
            // Assign skins
            if (node->skinIndex > -1) {
                node->skin = skins[node->skinIndex];
            }
            // Initial pose
            if (node->mesh != nullptr) {
                node->update();
            }
        }
    } else {
        // TODO: throw
        utils::exitFatal(
            "Could not load glTF file \"" + filename + "\": " + error, -1);
        return;
    }

    // Pre-Calculations for requested features
    if (static_cast<bool>(fileLoadingFlags &
                          FileLoadingFlags::PreTransformVertices) ||
        static_cast<bool>(fileLoadingFlags &
                          FileLoadingFlags::PreMultiplyVertexColors) ||
        static_cast<bool>(fileLoadingFlags & FileLoadingFlags::FlipY)) {
        const bool preTransform = static_cast<bool>(
            fileLoadingFlags & FileLoadingFlags::PreTransformVertices);
        const bool preMultiplyColor = static_cast<bool>(
            fileLoadingFlags & FileLoadingFlags::PreMultiplyVertexColors);
        const bool flipY =
            static_cast<bool>(fileLoadingFlags & FileLoadingFlags::FlipY);
        for (Node *node : linearNodes) {
            if (node->mesh != nullptr) {
                const glm::mat4 localMatrix = node->getMatrix();
                for (Primitive *primitive : node->mesh->primitives) {
                    for (uint32_t i = 0; i < primitive->vertexCount; i++) {
                        Vertex &vertex =
                            vertexBuffer[primitive->firstVertex + i];
                        // Pre-transform vertex positions by node-hierarchy
                        if (preTransform) {
                            vertex.pos = glm::vec3(localMatrix *
                                                   glm::vec4(vertex.pos, 1.0F));
                            vertex.normal = glm::normalize(
                                glm::mat3(localMatrix) * vertex.normal);
                        }
                        // Flip Y-Axis of vertex positions
                        if (flipY) {
                            vertex.pos.y *= -1.0F;
                            vertex.normal.y *= -1.0F;
                        }
                        // Pre-Multiply vertex colors with material base color
                        if (preMultiplyColor) {
                            vertex.color = primitive->material.baseColorFactor *
                                           vertex.color;
                        }
                    }
                }
            }
        }
    }

    for (const auto &extension : gltfModel.extensionsUsed) {
        if (extension == "KHR_materials_pbrSpecularGlossiness") {
            std::cout << "Required extension: " << extension;
            metallicRoughnessWorkflow = false;
        }
    }

    size_t vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);
    size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
    indices.count = static_cast<uint32_t>(indexBuffer.size());
    vertices.count = static_cast<uint32_t>(vertexBuffer.size());

    assert((vertexBufferSize > 0) && (indexBufferSize > 0));

    struct StagingBuffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
    } vertexStaging, indexStaging;

    // Create staging buffers
    // Vertex data
    VK_CHECK(m_deviceHandler->createBuffer(
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBufferSize, &vertexStaging.buffer, &vertexStaging.memory,
        vertexBuffer.data()));
    // Index data
    VK_CHECK(m_deviceHandler->createBuffer(
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        indexBufferSize, &indexStaging.buffer, &indexStaging.memory,
        indexBuffer.data()));

    // Create device local buffers
    // Vertex buffer
    VK_CHECK(m_deviceHandler->createBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            memoryPropertyFlags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBufferSize, &vertices.buffer,
        &vertices.memory, nullptr));

    // Index buffer
    VK_CHECK(m_deviceHandler->createBuffer(
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            memoryPropertyFlags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBufferSize, &indices.buffer,
        &indices.memory, nullptr));

    // Copy from staging buffers
    VkCommandBuffer copyCmd = m_commandBuffer->createCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkBufferCopy copyRegion = {};

    copyRegion.size = vertexBufferSize;
    vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, vertices.buffer, 1,
                    &copyRegion);

    copyRegion.size = indexBufferSize;
    vkCmdCopyBuffer(copyCmd, indexStaging.buffer, indices.buffer, 1,
                    &copyRegion);

    m_commandBuffer->flushCommandBuffer(copyCmd, transferQueue, true);

    vkDestroyBuffer(*m_deviceHandler, vertexStaging.buffer, nullptr);
    vkFreeMemory(*m_deviceHandler, vertexStaging.memory, nullptr);
    vkDestroyBuffer(*m_deviceHandler, indexStaging.buffer, nullptr);
    vkFreeMemory(*m_deviceHandler, indexStaging.memory, nullptr);

    getSceneDimensions();

    // Setup descriptors
    uint32_t uboCount{0};
    uint32_t imageCount{0};
    for (auto *node : linearNodes) {
        if (node->mesh != nullptr) {
            uboCount++;
        }
    }
    for (auto &material : materials) {
        if (material.baseColorTexture != nullptr) {
            imageCount++;
        }
    }
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uboCount},
    };
    if (imageCount > 0) {
        if (static_cast<bool>(descriptorBindingFlags &
                              DescriptorBindingFlags::ImageBaseColor)) {
            poolSizes.push_back(
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount});
        }
        if (static_cast<bool>(descriptorBindingFlags &
                              DescriptorBindingFlags::ImageNormalMap)) {
            poolSizes.push_back(
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount});
        }
    }
    VkDescriptorPoolCreateInfo descriptorPoolCI{};
    descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolCI.pPoolSizes = poolSizes.data();
    descriptorPoolCI.maxSets = uboCount + imageCount;
    VK_CHECK(vkCreateDescriptorPool(*m_deviceHandler, &descriptorPoolCI,
                                    nullptr, &descriptorPool));

    // Descriptors for per-node uniform buffers
    {
        // Layout is global, so only create if it hasn't already been created
        // before
        if (descriptorSetLayoutUbo == VK_NULL_HANDLE) {
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                create_info::descriptorSetLayoutBinding(
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    VK_SHADER_STAGE_VERTEX_BIT, 0, 1),
            };
            VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
            descriptorLayoutCI.sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorLayoutCI.bindingCount =
                static_cast<uint32_t>(setLayoutBindings.size());
            descriptorLayoutCI.pBindings = setLayoutBindings.data();
            VK_CHECK(vkCreateDescriptorSetLayout(*m_deviceHandler,
                                                 &descriptorLayoutCI, nullptr,
                                                 &descriptorSetLayoutUbo));
        }
        for (auto *node : nodes) {
            prepareNodeDescriptor(node, descriptorSetLayoutUbo);
        }
    }

    // Descriptors for per-material images
    {
        // Layout is global, so only create if it hasn't already been created
        // before
        if (descriptorSetLayoutImage == VK_NULL_HANDLE) {
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
            if (static_cast<bool>(descriptorBindingFlags &
                                  DescriptorBindingFlags::ImageBaseColor)) {
                setLayoutBindings.push_back(
                    create_info::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        static_cast<uint32_t>(setLayoutBindings.size()), 1));
            }
            if (static_cast<bool>(descriptorBindingFlags &
                                  DescriptorBindingFlags::ImageNormalMap)) {
                setLayoutBindings.push_back(
                    create_info::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        static_cast<uint32_t>(setLayoutBindings.size()), 1));
            }
            VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
            descriptorLayoutCI.sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorLayoutCI.bindingCount =
                static_cast<uint32_t>(setLayoutBindings.size());
            descriptorLayoutCI.pBindings = setLayoutBindings.data();
            VK_CHECK(vkCreateDescriptorSetLayout(*m_deviceHandler,
                                                 &descriptorLayoutCI, nullptr,
                                                 &descriptorSetLayoutImage));
        }
        for (auto &material : materials) {
            if (material.baseColorTexture != nullptr) {
                material.createDescriptorSet(
                    descriptorPool, gltf_model::descriptorSetLayoutImage,
                    descriptorBindingFlags);
            }
        }
    }
}

void gltf_model::Model::bindBuffers(VkCommandBuffer commandBuffer) {
    const std::array<VkDeviceSize, 1> offsets = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer,
                           offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0,
                         VK_INDEX_TYPE_UINT32);
    buffersBound = true;
}

void gltf_model::Model::drawNode(Node *node, VkCommandBuffer commandBuffer,
                                 uint32_t renderFlags,
                                 VkPipelineLayout pipelineLayout,
                                 uint32_t bindImageSet) {
    if (node->mesh != nullptr) {
        for (Primitive *primitive : node->mesh->primitives) {
            bool skip = false;
            const gltf_model::Material &material = primitive->material;
            if (static_cast<bool>(renderFlags &
                                  RenderFlags::RenderOpaqueNodes)) {
                skip = (material.alphaMode != Material::ALPHAMODE_OPAQUE);
            }
            if (static_cast<bool>(renderFlags &
                                  RenderFlags::RenderAlphaMaskedNodes)) {
                skip = (material.alphaMode != Material::ALPHAMODE_MASK);
            }
            if (static_cast<bool>(renderFlags &
                                  RenderFlags::RenderAlphaBlendedNodes)) {
                skip = (material.alphaMode != Material::ALPHAMODE_BLEND);
            }
            if (!skip) {
                if (static_cast<bool>(renderFlags & RenderFlags::BindImages)) {
                    vkCmdBindDescriptorSets(
                        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipelineLayout, bindImageSet, 1,
                        &material.descriptorSet, 0, nullptr);
                }
                vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1,
                                 primitive->firstIndex, 0, 0);
            }
        }
    }
    for (auto &child : node->children) {
        drawNode(child, commandBuffer, renderFlags, pipelineLayout,
                 bindImageSet);
    }
}

void gltf_model::Model::draw(VkCommandBuffer commandBuffer,
                             uint32_t renderFlags,
                             VkPipelineLayout pipelineLayout,
                             uint32_t bindImageSet) {
    if (!buffersBound) {
        const std::array<VkDeviceSize, 1> offsets = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer,
                               offsets.data());
        vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0,
                             VK_INDEX_TYPE_UINT32);
    }
    for (auto &node : nodes) {
        drawNode(node, commandBuffer, renderFlags, pipelineLayout,
                 bindImageSet);
    }
}

void gltf_model::Model::getNodeDimensions(Node *node, glm::vec3 &min,
                                          glm::vec3 &max) {
    if (node->mesh != nullptr) {
        for (Primitive *primitive : node->mesh->primitives) {
            glm::vec4 locMin =
                glm::vec4(primitive->dimensions.min, 1.0F) * node->getMatrix();
            glm::vec4 locMax =
                glm::vec4(primitive->dimensions.max, 1.0F) * node->getMatrix();
            if (locMin.x < min.x) {
                min.x = locMin.x;
            }
            if (locMin.y < min.y) {
                min.y = locMin.y;
            }
            if (locMin.z < min.z) {
                min.z = locMin.z;
            }
            if (locMax.x > max.x) {
                max.x = locMax.x;
            }
            if (locMax.y > max.y) {
                max.y = locMax.y;
            }
            if (locMax.z > max.z) {
                max.z = locMax.z;
            }
        }
    }
    for (auto *child : node->children) {
        getNodeDimensions(child, min, max);
    }
}

void gltf_model::Model::getSceneDimensions() {
    dimensions.min = glm::vec3(FLT_MAX);
    dimensions.max = glm::vec3(-FLT_MAX);
    for (auto *node : nodes) {
        getNodeDimensions(node, dimensions.min, dimensions.max);
    }
    dimensions.size = dimensions.max - dimensions.min;
    dimensions.center = (dimensions.min + dimensions.max) / 2.0F;
    dimensions.radius = glm::distance(dimensions.min, dimensions.max) / 2.0F;
}

void gltf_model::Model::updateAnimation(uint32_t index, float time) {
    if (index > static_cast<uint32_t>(animations.size()) - 1) {
        std::cout << "No animation with index " << index << std::endl;
        return;
    }
    Animation &animation = animations[index];

    bool updated = false;
    for (auto &channel : animation.channels) {
        gltf_model::AnimationSampler &sampler =
            animation.samplers[channel.samplerIndex];
        if (sampler.inputs.size() > sampler.outputsVec4.size()) {
            continue;
        }

        for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
            if ((time >= sampler.inputs[i]) &&
                (time <= sampler.inputs[i + 1])) {
                float interpCoef = std::max(0.0F, time - sampler.inputs[i]) /
                                   (sampler.inputs[i + 1] - sampler.inputs[i]);
                if (interpCoef <= 1.0F) {
                    switch (channel.path) {
                    case gltf_model::AnimationChannel::PathType::TRANSLATION: {
                        glm::vec4 trans =
                            glm::mix(sampler.outputsVec4[i],
                                     sampler.outputsVec4[i + 1], interpCoef);
                        channel.node->translation = glm::vec3(trans);
                        break;
                    }
                    case gltf_model::AnimationChannel::PathType::SCALE: {
                        glm::vec4 trans =
                            glm::mix(sampler.outputsVec4[i],
                                     sampler.outputsVec4[i + 1], interpCoef);
                        channel.node->scale = glm::vec3(trans);
                        break;
                    }
                    case gltf_model::AnimationChannel::PathType::ROTATION: {
                        glm::quat quad1;
                        quad1.x = sampler.outputsVec4[i].x;
                        quad1.y = sampler.outputsVec4[i].y;
                        quad1.z = sampler.outputsVec4[i].z;
                        quad1.w = sampler.outputsVec4[i].w;
                        glm::quat quad2;
                        quad2.x = sampler.outputsVec4[i + 1].x;
                        quad2.y = sampler.outputsVec4[i + 1].y;
                        quad2.z = sampler.outputsVec4[i + 1].z;
                        quad2.w = sampler.outputsVec4[i + 1].w;
                        channel.node->rotation = glm::normalize(
                            glm::slerp(quad1, quad2, interpCoef));
                        break;
                    }
                    }
                    updated = true;
                }
            }
        }
    }
    if (updated) {
        for (auto &node : nodes) {
            node->update();
        }
    }
}

/*
    Helper functions
*/
gltf_model::Node *gltf_model::Model::findNode(Node *parent, uint32_t index) {
    Node *nodeFound = nullptr;
    if (parent->index == index) {
        return parent;
    }
    for (auto &child : parent->children) {
        nodeFound = findNode(child, index);
        if (nodeFound != nullptr) {
            break;
        }
    }
    return nodeFound;
}

gltf_model::Node *gltf_model::Model::nodeFromIndex(uint32_t index) {
    Node *nodeFound = nullptr;
    for (auto &node : nodes) {
        nodeFound = findNode(node, index);
        if (nodeFound != nullptr) {
            break;
        }
    }
    return nodeFound;
}

void gltf_model::Model::prepareNodeDescriptor(
    gltf_model::Node *node, VkDescriptorSetLayout descriptorSetLayout) {
    if (node->mesh != nullptr) {
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool = descriptorPool;
        descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
        descriptorSetAllocInfo.descriptorSetCount = 1;
        VK_CHECK(
            vkAllocateDescriptorSets(*m_deviceHandler, &descriptorSetAllocInfo,
                                     &node->mesh->uniformBuffer.descriptorSet));

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.dstSet = node->mesh->uniformBuffer.descriptorSet;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.pBufferInfo = &node->mesh->uniformBuffer.descriptor;

        vkUpdateDescriptorSets(*m_deviceHandler, 1, &writeDescriptorSet, 0,
                               nullptr);
    }
    for (auto &child : node->children) {
        prepareNodeDescriptor(child, descriptorSetLayout);
    }
}
