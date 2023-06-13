#pragma once

#include "gltf_model/gltf_common.hpp"
#include "gltf_model/mesh.hpp"
#include "gltf_model/primitive.hpp"
#include "gltf_model/skin.hpp"

namespace gltf_model {
/*
    glTF node
*/
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
    [[nodiscard]] glm::mat4 localMatrix() const;
    [[nodiscard]] glm::mat4 getMatrix() const;
    void update();
    ~Node();
};

} // namespace gltf_model
