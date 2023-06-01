#pragma once
#include "common.hpp"
#include "geometry/uniform_buffer_objects.hpp"

namespace geometry {
static const float HALF_ROTATION = 180.0F; /**< A constant. 180 degrees */
static const float FULL_ROTATION = 360.0F; /**< A constant. 360 degrees */

/* Default camera properties */
static const float ROTATION_SENSITIVITY = 2.0F; /**< Default cam sensitivity*/
static const float DEFAULT_FOCUS = 10.0F;       /**< Default focus length */
static const float DEFAULT_FOV = 90.0F;         /**< Default field of view */

/**
 * \brief Represents a camera's position in 3D space
 *
 * This struct contains the information about the caera's
 * position and rotation, and caluclates the direction and up
 * vectors based on that
 */

struct Camera {
    float fov = DEFAULT_FOV;                  /**< Field of view in degrees */
    float focus = DEFAULT_FOCUS;              /**< focus length */
    float sensitivity = ROTATION_SENSITIVITY; /**< Camera sensitivity */

    float rotationVert = 0.0F;  /**< Current pitch */
    float rotationHoriz = 0.0F; /**< Current yaw */

    glm::vec3 position = {0.0F, 0.0F, 0.0F};  /**< Position vector */
    glm::vec3 direction = {0.0F, 0.0F, 0.0F}; /**< View direction vector */
    glm::vec3 up = {0.0F, 1.0F, 0.0F};        /**< Up vector */

    /**
     * \fn TransformMatrices transformMatrices(float width, float height)
     *
     * \brief - Calculate tranform matrices for given screen size
     *
     * \param width - the screen width
     * \param height - the screen height
     *
     * \return The transform matrices for current camera position
     */
    [[nodiscard]] TransformMatrices transformMatrices(float width,
                                                      float height) const;

    /**
     * \fn void moveForward(float timePassed)
     *
     * \brief Move camera forward by timePassed * direction
     *
     * \param timePassed - the time between frames
     */
    void moveForward(float timePassed);

    /**
     * \fn void moveLeft(float timePassed)
     *
     * \brief Move camera left by timePassed * cross(direction, up)
     *
     * \param timePassed - the time between frames
     */
    void moveLeft(float timePassed);

    /**
     * \fn void moveUp(float timePassed)
     *
     * \brief Move camera forward by timePassed * up
     *
     * \param timePassed - the time between frames
     */
    void moveUp(float timePassed);

    /**
     * \fn void calcTurn(float dHoriz, float dVert)
     *
     * \brief calculate the camera turn
     *
     * \param dHoriz - the horizontal turn
     * \param dVert - the vertical
     */
    void calcTurn(float dHoriz, float dVert);

    /**
     * \fn void calcRotation(float dHoriz, float dVert)
     *
     * \brief calculate the camera rotation at set degrees
     *
     * \param dHoriz - the horizontal turn
     * \param dVert - the vertical
     */
    void calcRotation(float dHoriz, float dVert);
};
} // namespace geometry
