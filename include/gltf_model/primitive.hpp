#pragma once

#include "gltf_model/gltf_common.hpp"
#include "gltf_model/material.hpp"

namespace gltf_model {
/*
    glTF primitive
*/
struct Primitive {
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t firstVertex;
    uint32_t vertexCount;
    Material &material;

    struct Dimensions {
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
        glm::vec3 size;
        glm::vec3 center;
        float radius;
    } dimensions;

    void setDimensions(glm::vec3 min, glm::vec3 max);
    Primitive(uint32_t firstIndex, uint32_t indexCount, Material &material)
        : firstIndex(firstIndex), indexCount(indexCount), material(material){};
};

} // namespace gltf_model
