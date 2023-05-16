#pragma once
#include "common.hpp"

namespace window {
GLFWwindow *initWindow(GLFWframebuffersizefun framebufferResizeCallback,
                       void *pUserData);
} // namespace window
