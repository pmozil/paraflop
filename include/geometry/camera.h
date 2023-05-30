#pragma once
#include "common.hpp"
#include "geometry/uniform_buffer_object.hpp"

namespace geometry {
const float FOCAL_LEN = 0.1F;
const float FOCUS = 10.0F;
const float FOV = glm::radians(90.0F);

struct Camera {
    float fov = FOV;
    float focus = FOCUS;
    float focal_len = FOCAL_LEN;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 up;

    [[nodiscard]] inline UniformBufferObject
    transformMatrices(float width, float height) const {
        UniformBufferObject ubo{};

        ubo.model = glm::lookAt(position, position + rotation, up);
        ubo.view = glm::translate(glm::mat4(1.0F), -position);
        ubo.proj = glm::perspective(fov, width / height, focal_len, focus);

        return ubo;
    }
};
} // namespace geometry
