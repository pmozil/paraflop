/**
 * Vulkan glTF model and texture loading class based on tinyglTF
 * (https://github.com/syoyo/tinygltf)
 *
 * Copyright (C) 2018-2022 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT)
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include "common.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace gltf_model {
struct Node;

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
    bool valid = false;
    BoundingBox();
    BoundingBox(glm::vec3 min, glm::vec3 max);
    BoundingBox getAABB(glm::mat4 mat);
};

struct TextureSampler {
    VkFilter magFilter;
    VkFilter minFilter;
    VkSamplerAddressMode addressModeU;
    VkSamplerAddressMode addressModeV;
    VkSamplerAddressMode addressModeW;
};

struct Texture {
    std::shared_ptr<device::DeviceHandler> deviceHandler;
    std::shared_ptr<command_buffer::CommandBufferHandler> commandBufferHandler;
    VkImage image;
    VkImageLayout imageLayout;
    VkDeviceMemory deviceMemory;
    VkImageView view;
    uint32_t width, height;
    uint32_t mipLevels;
    uint32_t layerCount;
    VkDescriptorImageInfo descriptor;
    VkSampler sampler;
    void updateDescriptor();
    void destroy();
    // Load a texture from a glTF image (stored as vector of chars loaded via
    // stb_image) and generate a full mip chaing for it
    void
    fromglTfImage(tinygltf::Image &gltfimage, TextureSampler textureSampler,
                  std::shared_ptr<device::DeviceHandler> device,
                  std::shared_ptr<command_buffer::CommandBufferHandler> cmdBuf,
                  VkQueue copyQueue);
};

struct Material {
    enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
    AlphaMode alphaMode = ALPHAMODE_OPAQUE;
    float alphaCutoff = 1.0F;
    float metallicFactor = 1.0F;
    float roughnessFactor = 1.0F;
    glm::vec4 baseColorFactor = glm::vec4(1.0F);
    glm::vec4 emissiveFactor = glm::vec4(1.0F);
    gltf_model::Texture *baseColorTexture;
    gltf_model::Texture *metallicRoughnessTexture;
    gltf_model::Texture *normalTexture;
    gltf_model::Texture *occlusionTexture;
    gltf_model::Texture *emissiveTexture;
    bool doubleSided = false;
    struct TexCoordSets {
        uint8_t baseColor = 0;
        uint8_t metallicRoughness = 0;
        uint8_t specularGlossiness = 0;
        uint8_t normal = 0;
        uint8_t occlusion = 0;
        uint8_t emissive = 0;
    } texCoordSets;
    struct Extension {
        gltf_model::Texture *specularGlossinessTexture;
        gltf_model::Texture *diffuseTexture;
        glm::vec4 diffuseFactor = glm::vec4(1.0F);
        glm::vec3 specularFactor = glm::vec3(0.0F);
    } extension;
    struct PbrWorkflows {
        bool metallicRoughness = true;
        bool specularGlossiness = false;
    } pbrWorkflows;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};

struct Primitive {
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t vertexCount;
    Material &material;
    bool hasIndices;
    BoundingBox bb;
    Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount,
              Material &material);
    void setBoundingBox(glm::vec3 min, glm::vec3 max);
};

struct Mesh {
    std::shared_ptr<device::DeviceHandler> deviceHandler;
    std::shared_ptr<command_buffer::CommandBufferHandler> commandBufferHandler;
    std::vector<Primitive *> primitives;
    BoundingBox bb;
    BoundingBox aabb;
    struct UniformBuffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
        VkDescriptorBufferInfo descriptor;
        VkDescriptorSet descriptorSet;
        void *mapped;
    } uniformBuffer;

    struct UniformBlock {
        glm::mat4 matrix;
        glm::mat4 jointMatrix[MAX_JOINTS]{};
        float jointcount{0};
    } uniformBlock;
    Mesh(std::shared_ptr<device::DeviceHandler> m_deviceHandler,
         std::shared_ptr<command_buffer::CommandBufferHandler>
             m_commandBufferHandler,
         glm::mat4 matrix);
    ~Mesh();
    void setBoundingBox(glm::vec3 min, glm::vec3 max);
};

struct Skin {
    std::string name;
    Node *skeletonRoot = nullptr;
    std::vector<glm::mat4> inverseBindMatrices;
    std::vector<Node *> joints;
};

struct Node {
    Node *parent;
    uint32_t index;
    std::vector<Node *> children;
    glm::mat4 matrix;
    std::string name;
    Mesh *mesh;
    Skin *skin;
    int32_t skinIndex = -1;
    glm::vec3 translation{};
    glm::vec3 scale{1.0F};
    glm::quat rotation{};
    BoundingBox bvh;
    BoundingBox aabb;
    glm::mat4 localMatrix();
    glm::mat4 getMatrix();
    void update();
    ~Node();
};

struct AnimationChannel {
    enum PathType { TRANSLATION, ROTATION, SCALE };
    PathType path;
    Node *node;
    uint32_t samplerIndex;
};

struct AnimationSampler {
    enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
    InterpolationType interpolation;
    std::vector<float> inputs;
    std::vector<glm::vec4> outputsVec4;
};

struct Animation {
    std::string name;
    std::vector<AnimationSampler> samplers;
    std::vector<AnimationChannel> channels;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::min();
};

struct Model {

    std::shared_ptr<device::DeviceHandler> deviceHandler;
    std::shared_ptr<command_buffer::CommandBufferHandler> commandBufferHandler;

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv0;
        glm::vec2 uv1;
        glm::vec4 joint0;
        glm::vec4 weight0;
        glm::vec4 color;
    };

    struct Vertices {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory;
    } vertices;
    struct Indices {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory;
    } indices;

    glm::mat4 aabb;

    std::vector<Node *> nodes;
    std::vector<Node *> linearNodes;

    std::vector<Skin *> skins;

    std::vector<Texture> textures;
    std::vector<TextureSampler> textureSamplers;
    std::vector<Material> materials;
    std::vector<Animation> animations;
    std::vector<std::string> extensions;

    struct Dimensions {
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
    } dimensions;

    struct LoaderInfo {
        uint32_t *indexBuffer;
        Vertex *vertexBuffer;
        size_t indexPos = 0;
        size_t vertexPos = 0;
    };

    void destroy(VkDevice device);
    void loadNode(gltf_model::Node *parent, const tinygltf::Node &node,
                  uint32_t nodeIndex, const tinygltf::Model &model,
                  LoaderInfo &loaderInfo, float globalscale);
    void getNodeProps(const tinygltf::Node &node, const tinygltf::Model &model,
                      size_t &vertexCount, size_t &indexCount);
    void loadSkins(tinygltf::Model &gltfModel);
    void loadTextures(tinygltf::Model &gltfModel,
                      std::shared_ptr<device::DeviceHandler> m_deviceHandler,
                      std::shared_ptr<command_buffer::CommandBufferHandler>
                          m_commandBufferHandler,
                      VkQueue transferQueue);
    VkSamplerAddressMode getVkWrapMode(int32_t wrapMode);
    VkFilter getVkFilterMode(int32_t filterMode);
    void loadTextureSamplers(tinygltf::Model &gltfModel);
    void loadMaterials(tinygltf::Model &gltfModel);
    void loadAnimations(tinygltf::Model &gltfModel);
    void loadFromFile(std::string filename,
                      std::shared_ptr<device::DeviceHandler> m_deviceHandler,
                      std::shared_ptr<command_buffer::CommandBufferHandler>
                          m_commandBufferHandler,
                      VkQueue transferQueue, float scale = 1.0F);
    void drawNode(Node *node, VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);
    void calculateBoundingBox(Node *node, Node *parent);
    void getSceneDimensions();
    void updateAnimation(uint32_t index, float time);
    Node *findNode(Node *parent, uint32_t index);
    Node *nodeFromIndex(uint32_t index);
};
} // namespace gltf_model
