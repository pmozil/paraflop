#include "interface/glfw_callbacks.hpp"

void handleKeyPress(GLFWwindow *window, int key, int sancode, int action,
                    int mods) {
    UNUSED(sancode);
    UNUSED(action);
    UNUSED(mods);
    auto *cam = (CameraRotation *)glfwGetWindowUserPointer(window);
    if (key == GLFW_KEY_W) {
        cam->camera->moveForward(cam->timePassed);
    }
    if (key == GLFW_KEY_S) {
        cam->camera->moveForward(-cam->timePassed);
    }
    if (key == GLFW_KEY_A) {
        cam->camera->moveLeft(cam->timePassed);
    }
    if (key == GLFW_KEY_D) {
        cam->camera->moveLeft(-cam->timePassed);
    }
    if (key == GLFW_KEY_Q) {
        cam->camera->moveUp(cam->timePassed);
    }
    if (key == GLFW_KEY_E) {
        cam->camera->moveUp(-cam->timePassed);
    }
    if (key == GLFW_KEY_Y) {
        cam->camera->calcRotation(0.0F, 0.0F);
    }
}

void handleFocus(GLFWwindow *window, int focused) {
    double xpos;
    double ypos;
    int width;
    int height;

    glfwGetWindowSize(window, &width, &height);
    glfwGetCursorPos(window, &xpos, &ypos);

    auto *cam = (CameraRotation *)glfwGetWindowUserPointer(window);

    if (static_cast<bool>(focused)) {
        cam->mouseX = xpos / width;
        cam->mouseY = ypos / height;
    }
}

void handleCursor(GLFWwindow *window, double xpos, double ypos) {
    int width;
    int height;

    glfwGetWindowSize(window, &width, &height);

    auto *cam = (CameraRotation *)glfwGetWindowUserPointer(window);

    float relX = xpos / width;
    float relY = ypos / height;

    float diffX = cam->mouseX - relX;
    float diffY = relY - cam->mouseY;

    if ((cam->mouseX != 0.0 || cam->mouseY != 0.0) &&
        (diffX * diffX + diffY * diffY < 1)) {
        cam->camera->calcTurn(diffX * cam->camera->focus * cam->camera->focus,
                              diffY * cam->camera->focus * cam->camera->focus);
    }
    cam->mouseX = relX;
    cam->mouseY = relY;

    // glfwSetCursorPos(window, width / 2, height / 2);
}
