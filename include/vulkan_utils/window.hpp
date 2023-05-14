#pragma once
#include "vulkan_utils/common.hpp"

namespace window {
GLFWwindow *initWindow(GLFWframebuffersizefun framebufferResizeCallback,
                       void *pUserData);
} // namespace window
