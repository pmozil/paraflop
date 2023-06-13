#pragma once

#include "gltf_model/gltf_common.hpp"
#include "gltf_model/texture.hpp"

namespace gltf_model {
/*
    glTF material class
*/
struct Material {
    std::shared_ptr<device::DeviceHandler> deviceHandler;
    std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer;
    enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
    AlphaMode alphaMode = ALPHAMODE_OPAQUE;
    float alphaCutoff = 1.0F;
    float metallicFactor = 1.0F;
    float roughnessFactor = 1.0F;
    bool doubleSided = false;
    glm::vec4 baseColorFactor = glm::vec4(1.0F);
    gltf_model::Texture *baseColorTexture = nullptr;
    gltf_model::Texture *metallicRoughnessTexture = nullptr;
    gltf_model::Texture *normalTexture = nullptr;
    gltf_model::Texture *occlusionTexture = nullptr;
    gltf_model::Texture *emissiveTexture = nullptr;

    gltf_model::Texture *specularGlossinessTexture;
    gltf_model::Texture *diffuseTexture;

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    Material(std::shared_ptr<device::DeviceHandler> deviceHandler,
             std::shared_ptr<command_buffer::CommandBufferHandler> cmdBuf)
        : deviceHandler(std::move(deviceHandler)),
          commandBuffer(std::move(cmdBuf)){};
    void createDescriptorSet(VkDescriptorPool descriptorPool,
                             VkDescriptorSetLayout descriptorSetLayout,
                             uint32_t descriptorBindingFlags);
};
} // namespace gltf_model
