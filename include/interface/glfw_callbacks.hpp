#pragma once
#include "geometry/camera.hpp"

struct AppState {
    std::shared_ptr<geometry::Camera> camera;
    float timePassed = 0.0F;
    float mouseX = 0.0F;
    float mouseY = 0.0F;

    AppState(std::shared_ptr<geometry::Camera> cam) : camera(std::move(cam)) {}
};

void handleKeyPress(GLFWwindow *window, int key, int sancode, int action,
                    int mods);
void handleFocus(GLFWwindow *window, int focused);

void handleCursor(GLFWwindow *window, double xpos, double ypos);
