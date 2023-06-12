/*
 * Vulkan glTF model and texture loading class based on tinyglTF
 * (https://github.com/syoyo/tinygltf)
 *
 * Copyright (C) 2018 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT)
 * (http://opensource.org/licenses/MIT)
 */

#include "gltf_model/node.hpp"

namespace gltf_model {
/*
    glTF node
*/
[[nodiscard]] glm::mat4 gltf_model::Node::localMatrix() const {
    return glm::translate(glm::mat4(1.0F), translation) * glm::mat4(rotation) *
           glm::scale(glm::mat4(1.0F), scale) * matrix;
}

[[nodiscard]] glm::mat4 gltf_model::Node::getMatrix() const {
    glm::mat4 mat = localMatrix();
    gltf_model::Node *par = parent;
    while (par != nullptr) {
        mat = par->localMatrix() * mat;
        par = par->parent;
    }
    return mat;
}

void gltf_model::Node::update() {
    if (mesh != nullptr) {
        glm::mat4 mat = getMatrix();
        if (skin != nullptr) {
            mesh->uniformBlock.matrix = mat;
            // Update join matrices
            glm::mat4 inverseTransform = glm::inverse(mat);
            for (size_t i = 0; i < skin->joints.size(); i++) {
                gltf_model::Node *jointNode = skin->joints[i];
                glm::mat4 jointMat =
                    jointNode->getMatrix() * skin->inverseBindMatrices[i];
                jointMat = inverseTransform * jointMat;
                mesh->uniformBlock.jointMatrix[i] = jointMat;
            }
            mesh->uniformBlock.jointcount = (float)skin->joints.size();
            memcpy(mesh->uniformBuffer.mapped, &mesh->uniformBlock,
                   sizeof(mesh->uniformBlock));
        } else {
            memcpy(mesh->uniformBuffer.mapped, &mat, sizeof(glm::mat4));
        }
    }

    for (auto &child : children) {
        child->update();
    }
}

gltf_model::Node::~Node() {
    delete mesh;
    for (auto &child : children) {
        delete child;
    }
}

} // namespace gltf_model
