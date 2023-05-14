#include "vulkan_utils/window.hpp"
#include <GLFW/glfw3.h>

GLFWwindow *window::initWindow(GLFWframebuffersizefun framebufferResizeCallback,
                               void *pUsserData) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window;

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    if (pUsserData != nullptr) {
        glfwSetWindowUserPointer(window, pUsserData);
    }
    if (framebufferResizeCallback != nullptr) {
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }
    return window;
}
