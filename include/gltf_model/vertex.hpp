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

#include "gltf_model/gltf_common.hpp"

namespace gltf_model {
/*
    glTF default vertex layout with easy Vulkan mapping functions
*/
enum class VertexComponent {
    Position,
    Normal,
    UV,
    Color,
    Tangent,
    Joint0,
    Weight0
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 color;
    glm::vec4 joint0;
    glm::vec4 weight0;
    glm::vec4 tangent;
    static VkVertexInputBindingDescription vertexInputBindingDescription;
    static std::vector<VkVertexInputAttributeDescription>
        vertexInputAttributeDescriptions;
    static VkPipelineVertexInputStateCreateInfo
        pipelineVertexInputStateCreateInfo;
    static VkVertexInputBindingDescription
    inputBindingDescription(uint32_t binding);
    static VkVertexInputAttributeDescription
    inputAttributeDescription(uint32_t binding, uint32_t location,
                              VertexComponent component);
    static std::vector<VkVertexInputAttributeDescription>
    inputAttributeDescriptions(uint32_t binding,
                               std::vector<VertexComponent> components);
    /** @brief Returns the default pipeline vertex input state create info
     * structure for the requested vertex components */
    static VkPipelineVertexInputStateCreateInfo *
    getPipelineVertexInputState(std::vector<VertexComponent> components);
};
} // namespace gltf_model
