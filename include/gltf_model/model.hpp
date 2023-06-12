/*
 * Vulkan glTF model and texture loading class based on tinyglTF
 * (https://github.com/syoyo/tinygltf)
 *
 * Copyright (C) 2018 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT)
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include "gltf_model/animation.hpp"
#include "gltf_model/gltf_common.hpp"
#include "gltf_model/mesh.hpp"
#include "gltf_model/node.hpp"
#include "gltf_model/primitive.hpp"
#include "gltf_model/skin.hpp"
#include "gltf_model/texture.hpp"
#include "gltf_model/vertex.hpp"

namespace gltf_model {
/*
    glTF model loading and rendering class
*/
class Model {
  private:
    gltf_model::Texture *getTexture(uint32_t index);
    gltf_model::Texture emptyTexture;
    void createEmptyTexture(VkQueue transferQueue);

    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer;

  public:
    VkDescriptorPool descriptorPool;

    struct Vertices {
        int count;
        VkBuffer buffer;
        VkDeviceMemory memory;
    } vertices;

    struct Indices {
        int count;
        VkBuffer buffer;
        VkDeviceMemory memory;
    } indices;

    std::vector<Node *> nodes;
    std::vector<Node *> linearNodes;

    std::vector<Skin *> skins;

    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<Animation> animations;

    struct Dimensions {
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
        glm::vec3 size;
        glm::vec3 center;
        float radius;
    } dimensions;

    bool metallicRoughnessWorkflow = true;
    bool buffersBound = false;
    std::string path;

    Model() = default;
    ~Model();
    void loadNode(gltf_model::Node *parent, const tinygltf::Node &node,
                  uint32_t nodeIndex, const tinygltf::Model &model,
                  std::vector<uint32_t> &indexBuffer,
                  std::vector<Vertex> &vertexBuffer, float globalscale);
    void loadSkins(tinygltf::Model &gltfModel);
    void
    loadImages(tinygltf::Model &gltfModel,
               std::shared_ptr<device::DeviceHandler> device,
               std::shared_ptr<command_buffer::CommandBufferHandler> cmdBuf,
               VkQueue transferQueue);
    void loadMaterials(tinygltf::Model &gltfModel);
    void loadAnimations(tinygltf::Model &gltfModel);
    void
    loadFromFile(std::string filename,
                 std::shared_ptr<device::DeviceHandler> device,
                 std::shared_ptr<command_buffer::CommandBufferHandler> cmdBuf,
                 VkQueue transferQueue,
                 uint32_t fileLoadingFlags = gltf_model::FileLoadingFlags::None,
                 float scale = 1.0F);
    void bindBuffers(VkCommandBuffer commandBuffer);
    void drawNode(Node *node, VkCommandBuffer commandBuffer,
                  uint32_t renderFlags = 0,
                  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE,
                  uint32_t bindImageSet = 1);
    void draw(VkCommandBuffer commandBuffer, uint32_t renderFlags = 0,
              VkPipelineLayout pipelineLayout = VK_NULL_HANDLE,
              uint32_t bindImageSet = 1);
    void getNodeDimensions(Node *node, glm::vec3 &min, glm::vec3 &max);
    void getSceneDimensions();
    void updateAnimation(uint32_t index, float time);
    Node *findNode(Node *parent, uint32_t index);
    Node *nodeFromIndex(uint32_t index);
    void prepareNodeDescriptor(gltf_model::Node *node,
                               VkDescriptorSetLayout descriptorSetLayout);
};
} // namespace gltf_model
