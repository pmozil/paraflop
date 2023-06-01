#pragma once
#include "common.hpp"

namespace window {
/** \fn GLFWwindow *initWindow(GLFWframebuffersizefun framebufferResizeCallback,
                       GLFWkeyfun keyCallback, GLFWcursorposfun cursorCallback,
                       GLFWwindowfocusfun focusCallback, void *pUserData)

    \brief Create a GLFW window

    \param framebufferResizeCallback the callback for window resize
    \param keyCallback the callback for keypress
    \param cursorCallback the callback for cursor movement
    \param focusCallback the callback for window focus/unfocus
    \param pUserData the pointer to user data

    \return A pointer to a GLFW window handle
*/
GLFWwindow *initWindow(GLFWframebuffersizefun framebufferResizeCallback,
                       GLFWkeyfun keyCallback, GLFWcursorposfun cursorCallback,
                       GLFWwindowfocusfun focusCallback, void *pUserData);
} // namespace window
