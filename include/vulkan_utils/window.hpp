#pragma once
#include "common.hpp"

namespace window {
GLFWwindow *initWindow(GLFWframebuffersizefun framebufferResizeCallback,
                       GLFWkeyfun keyCallback, GLFWcursorposfun cursorCallback,
                       void *pUserData);
} // namespace window
