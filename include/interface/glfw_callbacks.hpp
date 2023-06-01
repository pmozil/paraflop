#pragma once
#include "geometry/camera.hpp"

/**
 * \struct AppState
 *
 * \brief Represents the state of the application.
 *
 * The AppState struct holds various data related to the application's state,
 * including the camera, time passed, and mouse position.
 */
struct AppState {
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
     * \fn AppState(std::shared_ptr<geometry::Camera> cam)
     *
     * \brief Constructs an AppState object.
     *
     * \param cam The camera associated with the application.
     */
    AppState(std::shared_ptr<geometry::Camera> cam) : camera(std::move(cam)) {}
};

/**
 * \fn void handleKeyPress(GLFWwindow *window, int key, int sancode, int action,
 * int mods)
 *
 * \brief Handles key press events.
 *
 * \param window The GLFW window.
 * \param key The key that was pressed.
 * \param sancode The system-specific scancode of the key.
 * \param action The action that was performed (press, release, repeat).
 * \param mods Bit field describing which modifier keys were held down.
 */
void handleKeyPress(GLFWwindow *window, int key, int sancode, int action,
                    int mods);

/**
 * \fn void handleFocus(GLFWwindow *window, int focused)
 *
 * \brief Handles focus events.
 *
 * \param window The GLFW window.
 * \param focused Indicates whether the window has gained or lost focus.
 */
void handleFocus(GLFWwindow *window, int focused);

/**
 * \fn void handleCursor(GLFWwindow *window, double xpos, double ypos)
 *
 * \brief Handles cursor position events.
 *
 * \param window The GLFW window.
 * \param xpos The new X position of the cursor.
 * \param ypos The new Y position of the cursor.
 */
void handleCursor(GLFWwindow *window, double xpos, double ypos);
