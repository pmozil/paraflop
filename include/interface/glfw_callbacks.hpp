#pragma once
#include "geometry/camera.hpp"
#include "interface/camera_rotation.hpp"

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
