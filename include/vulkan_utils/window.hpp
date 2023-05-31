#pragma once
#include "common.hpp"

namespace window {
GLFWwindow *initWindow(GLFWframebuffersizefun framebufferResizeCallback,
                       GLFWkeyfun keyCallback, void *pUserData);
} // namespace window
