#include "gltf_model/mesh.hpp"

namespace gltf_model {
/*
    glTF mesh
*/
gltf_model::Mesh::Mesh(
    std::shared_ptr<device::DeviceHandler> device,
    std::shared_ptr<command_buffer::CommandBufferHandler> cmdBuf,
    glm::mat4 matrix) {
    this->deviceHandler = std::move(device);
    this->commandBuffer = std::move(cmdBuf);
    this->uniformBlock.matrix = matrix;

    VK_CHECK(
        deviceHandler->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    sizeof(uniformBlock), &uniformBuffer.buffer,
                                    &uniformBuffer.memory, &uniformBlock));

    VK_CHECK(vkMapMemory(*deviceHandler, uniformBuffer.memory, 0,
                         sizeof(uniformBlock), 0, &uniformBuffer.mapped));
    uniformBuffer.descriptor = {uniformBuffer.buffer, 0, sizeof(uniformBlock)};
};

gltf_model::Mesh::~Mesh() {
    vkDestroyBuffer(*deviceHandler, uniformBuffer.buffer, nullptr);
    vkFreeMemory(*deviceHandler, uniformBuffer.memory, nullptr);
    for (auto *primitive : primitives) {
        delete primitive;
    }
}
} // namespace gltf_model
