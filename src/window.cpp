#include "vulkan_utils/window.hpp"
#include <GLFW/glfw3.h>

GLFWwindow *window::initWindow(GLFWframebuffersizefun framebufferResizeCallback,
                               void *pUserData) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window;

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    if (pUserData != nullptr) {
        glfwSetWindowUserPointer(window, pUserData);
    }
    if (framebufferResizeCallback != nullptr) {
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }
    return window;
}
