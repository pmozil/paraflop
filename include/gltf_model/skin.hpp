#pragma once

#include "gltf_model/gltf_common.hpp"
#include "gltf_model/material.hpp"
#include "gltf_model/primitive.hpp"
#include "gltf_model/texture.hpp"

namespace gltf_model {
/*
    glTF skin
*/
struct Skin {
    std::string name;
    Node *skeletonRoot = nullptr;
    std::vector<glm::mat4> inverseBindMatrices;
    std::vector<Node *> joints;
};
} // namespace gltf_model
