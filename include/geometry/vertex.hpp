#pragma once
#include "common.hpp"

namespace geometry {
/**
 * \brief Represents a vertex in a 3D geometry.
 *
 * This struct defines a vertex with position and color attributes.
 * It is used to describe the properties of individual vertices in a 3D mesh.
 */
struct Vertex {
    glm::vec3 pos;   /**< Position of the vertex */
    glm::vec3 color; /**< Color of the vertex */

    /**
     * \brief Retrieves the binding description for vertex input.
     *
     * This function returns a `VkVertexInputBindingDescription` struct
     * that describes the binding of vertex data for the graphics pipeline.
     *
     * \return The binding description for the vertex input.
     */
    static inline VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    /**
     * \brief Retrieves the attribute descriptions for vertex input.
     *
     * This function returns an array of `VkVertexInputAttributeDescription`
     * structs that describe the attributes of the vertex data for the graphics
     * pipeline.
     *
     * \return The attribute descriptions for the vertex input.
     */
    static inline std::array<VkVertexInputAttributeDescription, 2>
    getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2>
            attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};
} // namespace geometry
