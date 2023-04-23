#include "window.hpp"
#include <GLFW/glfw3.h>

void window::initWindow(GLFWwindow *window,
                        GLFWframebuffersizefun framebufferResizeCallback) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}
