#pragma once
#include "common.hpp"
#include "geometry/uniform_buffer_object.hpp"

namespace geometry {

static const float HALF_ROTATION = 180.0F;
static const float FULL_ROTATION = 360.0F;
static const float PI = 3.141592654;
static constexpr float INTO_RADIANS = 3.141592654 / 180.0F;

static const float ROTATION_SENSITIVITY = 2.0F;

/* Default camera properties */
static const float DEFAULT_FOCAL_LEN = 0.1F;
static const float DEFAULT_FOCUS = 10.0F;
static const float DEFAULT_FOV = glm::radians(90.0F);

struct Camera {
    float fov = DEFAULT_FOV;
    float focus = DEFAULT_FOCUS;
    float focal_len = DEFAULT_FOCAL_LEN;

    float rotationVert = 0.0F;
    float rotationHoriz = 0.0F;

    glm::vec3 position = {0.0F, 0.0F, 0.0F};
    glm::vec3 direction = {0.0F, 0.0F, 0.0F};
    glm::vec3 up = {0.0F, 1.0F, 0.0F};

    [[nodiscard]] UniformBufferObject transformMatrices(float width,
                                                        float height) const;

    void moveForward(float timePassed);

    void moveLeft(float timePassed);

    void moveUp(float timePassed);

    void calcTurn(float dVert, float dHoriz);

    void calcRotation(float dVert, float dHoriz);
};
} // namespace geometry
