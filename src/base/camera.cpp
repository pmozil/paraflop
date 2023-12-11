#include "geometry/camera.hpp"

namespace geometry {
[[nodiscard]] TransformMatrices Camera::transformMatrices(float width,
                                                          float height) const {
    TransformMatrices ubo{};

    ubo.model = glm::identity<glm::mat4>();
    ubo.view = glm::lookAt(position, position + direction, up);
    ubo.proj =
        glm::perspective(glm::radians(fov), width / height, 1 / focus, focus);

    return ubo;
}

void Camera::moveForward(float timePassed) {
    position += direction * timePassed * focus * 0.4F;
}

void Camera::moveLeft(float timePassed) {
    position += glm::cross(up, direction) * timePassed * focus * 0.4F;
}

void Camera::moveUp(float timePassed) {
    position -= up * timePassed * focus * 0.4F;
}

void Camera::calcTurn(float dHoriz, float dVert) {
    calcRotation(rotationVert + sensitivity * dHoriz,
                 rotationHoriz + sensitivity * dVert);
}

void Camera::calcRotation(float dHoriz, float dVert) {
    rotationVert = std::fmod(dHoriz, FULL_ROTATION);
    rotationHoriz = std::fmod(dVert, HALF_ROTATION);

    float vCos = std::cos(glm::radians(rotationVert));
    float vSin = std::sin(glm::radians(rotationVert));
    float hCos = std::cos(glm::radians(rotationHoriz));
    float hSin = std::sin(glm::radians(rotationHoriz));

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
    direction = {-vSin * hCos, -hSin, vCos * hCos};
    direction = glm::normalize(direction);
    up = {hSin * vSin, hCos, hSin * vCos};
    up = glm::normalize(-up);
}
} // namespace geometry
