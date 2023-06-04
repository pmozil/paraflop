#pragma once
#include "geometry/camera.hpp"

/**
 * \struct CameraRotation
 *
 * \brief Represents the state of the application.
 *
 * The CameraRotation struct holds various data related to the application's
 * state, including the camera, time passed, and mouse position.
 */
struct CameraRotation {
    /**
     * \var std::shared_ptr<geometry::Camera> camera
     *
     * \brief The camera associated with the application.
     */
    std::shared_ptr<geometry::Camera> camera;

    /**
     * \var float timePassed
     *
     * \brief The time passed in the application.
     */
    float timePassed = 0.0F;

    /**
     * \var float mouseX
     *
     * \brief The X position of the mouse cursor.
     */
    float mouseX = 0.0F;

    /**
     * \var float mouseY
     *
     * \brief The Y position of the mouse cursor.
     */
    float mouseY = 0.0F;

    /**
     * \fn CameraRotation(std::shared_ptr<geometry::Camera> cam)
     *
     * \brief Constructs an CameraRotation object.
     *
     * \param cam The camera associated with the application.
     */
    CameraRotation(std::shared_ptr<geometry::Camera> cam)
        : camera(std::move(cam)) {}
};
