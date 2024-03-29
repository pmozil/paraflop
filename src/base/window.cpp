#include "vulkan_utils/window.hpp"
#include <GLFW/glfw3.h>

GLFWwindow *window::initWindow(GLFWframebuffersizefun framebufferResizeCallback,
                               GLFWkeyfun keyCallback,
                               GLFWcursorposfun cursorCallback,
                               GLFWwindowfocusfun focusCallback,
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
    if (keyCallback != nullptr) {
        glfwSetKeyCallback(window, keyCallback);
    }
    if (cursorCallback != nullptr) {
        glfwSetCursorPosCallback(window, cursorCallback);
    }
    if (focusCallback != nullptr) {
        glfwSetWindowFocusCallback(window, focusCallback);
    }

    return window;
}
