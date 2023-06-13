#pragma once

#include "gltf_model/gltf_common.hpp"
#include "gltf_model/material.hpp"
#include "gltf_model/primitive.hpp"
#include "gltf_model/texture.hpp"

namespace gltf_model {
/*
    glTF mesh
*/
struct Mesh {
    std::shared_ptr<device::DeviceHandler> deviceHandler;
    std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer;

    std::vector<Primitive *> primitives;
    std::string name;

    struct UniformBuffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
        VkDescriptorBufferInfo descriptor;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        void *mapped;
    } uniformBuffer;

    struct UniformBlock {
        glm::mat4 matrix;
        std::array<glm::mat4, MAX_JOINTS> jointMatrix{};
        float jointcount{0};
    } uniformBlock;

    Mesh(std::shared_ptr<device::DeviceHandler> device,
         std::shared_ptr<command_buffer::CommandBufferHandler> cmdBuf,
         glm::mat4 matrix);
    ~Mesh();
};

} // namespace gltf_model
