#pragma once
#include "geometry/camera.hpp"

namespace geometry {
[[nodiscard]] UniformBufferObject
Camera::transformMatrices(float width, float height) const {
    UniformBufferObject ubo{};

    ubo.model = glm::identity<glm::mat4>();
    ubo.view = glm::lookAt(position, position + direction, up);
    ubo.proj = glm::perspective(fov, width / height, focal_len, focus);

    return ubo;
}

void Camera::moveForward(float timePassed) {
    position += direction * timePassed * focus;
}

void Camera::moveLeft(float timePassed) {
    position += glm::cross(up, direction) * timePassed * focus;
}

void Camera::moveUp(float timePassed) { position += up * timePassed * focus; }

void Camera::calcTurn(float dVert, float dHoriz) {
    calcRotation(std::fmod(rotationVert + dVert, FULL_ROTATION),
                 std::fmod(rotationHoriz + dHoriz, HALF_ROTATION));
}

void Camera::calcRotation(float dVert, float dHoriz) {
    rotationVert = std::fmod(dVert, FULL_ROTATION);
    rotationHoriz = std::fmod(dHoriz, HALF_ROTATION);

    float vCos = std::cos(rotationVert * INTO_RADIANS);
    float vSin = std::sin(rotationVert * INTO_RADIANS);
    float hCos = std::cos(rotationHoriz * INTO_RADIANS);
    float hSin = std::sin(rotationHoriz * INTO_RADIANS);

    /*
     * Spherical coordinates, baby
     * So to explain this idiocy:
     * We need the direction vector, AND
     * the up vector to be changed
     * depending on the camera direction.
     * Also, they have to be perpendicular.
     * ALSO also, the order of coordinates if fucked up:
     * it's (y, z, x) instead of  (x, y, z)
     *
     */
    direction = glm::normalize(glm::vec3({vSin * hCos, hSin, vCos * hCos}));
    up = glm::normalize(glm::cross(glm::cross(WORLD_UP, direction), direction));
}
} // namespace geometry
