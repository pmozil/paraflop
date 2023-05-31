#pragma once
#include "common.hpp"
#include "geometry/uniform_buffer_object.hpp"

namespace geometry {
const float FOCAL_LEN = 0.1F;
const float FOCUS = 10.0F;
const float FOV = glm::radians(90.0F);
const float HALF_ROTATION = 180.0F;
const float FULL_ROTATION = 360.0F;
const float PI = 3.141592654;
constexpr float INTO_RADIANS = 3.141592654 / 180.0F;

struct Camera {
    float fov = FOV;
    float focus = FOCUS;
    float focal_len = FOCAL_LEN;

    float rotationVert = 0.0F;
    float rotationHoriz = 0.0F;

    glm::vec3 position = {0.0F, 0.0F, 0.0F};
    glm::vec3 rotation = {0.0F, 0.0F, 0.0F};
    glm::vec3 up = {0.0F, 1.0F, 0.0F};

    [[nodiscard]] inline UniformBufferObject
    transformMatrices(float width, float height) const {
        UniformBufferObject ubo{};

        ubo.model = glm::identity<glm::mat4>();
        ubo.view = glm::lookAt(position, position + rotation, up);
        ubo.proj = glm::perspective(fov, width / height, focal_len, focus);

        return ubo;
    }

    inline void moveForward(float timePassed) {
        position += rotation * timePassed * focus;
    }

    inline void moveLeft(float timePassed) {
        position += glm::cross(up, rotation) * timePassed * focus;
    }

    inline void moveUp(float timePassed) {
        position -= glm::vec3({0.0, 1.0, 0.0}) * timePassed;
    }

    inline void calcTurn(float dVert, float dHoriz) {
        rotationVert = std::fmod(rotationVert + dVert, FULL_ROTATION);
        rotationHoriz = std::fmod(rotationHoriz + dHoriz, HALF_ROTATION);

        float vCos = std::cos(rotationVert * INTO_RADIANS);
        float vSin = std::sin(rotationVert * INTO_RADIANS);
        float hCos = std::cos(rotationHoriz * INTO_RADIANS);
        float hSin = std::sin(rotationHoriz * INTO_RADIANS);

        rotation = glm::normalize(glm::vec3({vSin * hCos, hSin, vCos * hCos}));
        up = glm::normalize(glm::vec3({hSin * vSin, vCos, -hCos * vSin}));
    }

    inline void calcRotation(float dVert, float dHoriz) {
        rotationVert = std::fmod(dVert, FULL_ROTATION);
        rotationHoriz = std::fmod(dHoriz, HALF_ROTATION);

        float vCos = std::cos(rotationVert * INTO_RADIANS);
        float vSin = std::sin(rotationVert * INTO_RADIANS);
        float hCos = std::cos(rotationHoriz * INTO_RADIANS);
        float hSin = std::sin(rotationHoriz * INTO_RADIANS);

        // y z x
        rotation = glm::normalize(glm::vec3({vSin * hCos, hSin, vCos * hCos}));
        up = glm::normalize(glm::vec3({hSin * vSin, vCos, -hCos * vSin}));
    }
};
} // namespace geometry
