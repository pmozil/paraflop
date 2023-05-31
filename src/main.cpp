#include "common.hpp"
#include "geometry/camera.h"
#include "geometry/uniform_buffer_object.hpp"
#include "geometry/vertex.hpp"
#include "interface/custom_graphics_pipeline.hpp"
#include "interface/renderer.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/debug.hpp"
#include "vulkan_utils/descriptor_set.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/graphics_pipeline.hpp"
#include "vulkan_utils/index_buffer.hpp"
#include "vulkan_utils/surface.hpp"
#include "vulkan_utils/swap_chain.hpp"
#include "vulkan_utils/vertex_buffer.hpp"
#include "vulkan_utils/vk_instance.hpp"
#include "vulkan_utils/window.hpp"

struct AppState {
    std::shared_ptr<geometry::Camera> camera;
    float timePassed = 0.0F;
    float mouseX = 0.0F;
    float mouseY = 0.0F;

    AppState(std::shared_ptr<geometry::Camera> cam) : camera(std::move(cam)) {}
};

const std::vector<geometry::Vertex> vertices = {
    {{-0.5F, -0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
    {{0.5F, -0.5F, 0.0F}, {0.0F, 1.0F, 0.0F}},
    {{0.5F, 0.5F, 0.0F}, {0.0F, 0.0F, 1.0F}},
    {{-0.5F, 0.5F, 0.0F}, {1.0F, 1.0F, 1.0F}}};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

void handleKeyPress(GLFWwindow *window, int key, int sancode, int action,
                    int mods) {
    auto *cam = (AppState *)glfwGetWindowUserPointer(window);
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
}

void handleFocus(GLFWwindow *window, int focused) {
    double xpos;
    double ypos;
    int width;
    int height;

    glfwGetWindowSize(window, &width, &height);
    glfwGetCursorPos(window, &xpos, &ypos);

    auto *cam = (AppState *)glfwGetWindowUserPointer(window);

    if (static_cast<bool>(focused)) {
        cam->mouseX = xpos / width;
        cam->mouseY = ypos / height;
    }
}

void handleCursor(GLFWwindow *window, double xpos, double ypos) {
    int width;
    int height;

    glfwGetWindowSize(window, &width, &height);

    auto *cam = (AppState *)glfwGetWindowUserPointer(window);

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

    glfwSetCursorPos(window, width / 2, height / 2);
}

int main() {
    std::vector<const char *> validation = {"VK_LAYER_KHRONOS_validation"};

    auto *cam = new AppState(nullptr);

    std::vector<const char *> devExt = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    GLFWwindow *window = window::initWindow(nullptr, handleKeyPress,
                                            handleCursor, handleFocus, cam);
    std::unique_ptr<vk_instance::Instance> instance =
        std::make_unique<vk_instance::Instance>();
    debug::createDebugMessenger(instance->instance);
    std::unique_ptr<surface::Surface> surface =
        std::make_unique<surface::Surface>(instance->instance, window, nullptr);
    std::shared_ptr<device::DeviceHandler> deviceHandler =
        std::make_shared<device::DeviceHandler>(
            devExt, validation, instance->instance, surface->surface);
    std::shared_ptr<swap_chain::SwapChain> swapChain =
        std::make_shared<swap_chain::SwapChain>(window, surface->surface,
                                                deviceHandler);

    VkDescriptorSetLayoutBinding uboBinding =
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0,
            1);

    std::shared_ptr<descriptor_set::DescriptorSetLayout> layout =
        std::make_shared<descriptor_set::DescriptorSetLayout>(deviceHandler,
                                                              &uboBinding, 1);

    std::shared_ptr<graphics_pipeline::CustomRasterPipeline> pipeline =
        std::make_shared<graphics_pipeline::CustomRasterPipeline>(
            swapChain, deviceHandler, &layout->layout);
    std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer =
        std::make_shared<command_buffer::CommandBufferHandler>(
            deviceHandler, swapChain, pipeline);

    std::shared_ptr<buffer::UniformBuffer> uniformBuffer =
        std::make_shared<buffer::UniformBuffer>(
            deviceHandler, commandBuffer,
            sizeof(geometry::UniformBufferObject));

    std::shared_ptr<geometry::Camera> camera =
        std::make_shared<geometry::Camera>();

    cam->camera = camera;

    glm::vec3 pos = {0.0F, 0.0F, 1.0F};

    camera->position = pos;

    camera->calcRotation(geometry::HALF_ROTATION, 0.0F);

    geometry::UniformBufferObject ubo = camera->transformMatrices(
        swapChain->swapChainExtent.width, swapChain->swapChainExtent.height);

    uniformBuffer->fastCopy((void *)&ubo, sizeof(ubo));

    static auto startTime = std::chrono::system_clock::now();
    auto prevTime = startTime;

    std::vector<buffer::UniformBuffer> buffs = {};

    std::shared_ptr<descriptor_set::DescriptorSetHandler> descriptorSets =
        std::make_shared<descriptor_set::DescriptorSetHandler>(
            deviceHandler, layout, uniformBuffer);

    std::shared_ptr<buffer::IndexBuffer> indexBuffer =
        std::make_shared<buffer::IndexBuffer>(
            deviceHandler, commandBuffer, sizeof(indices[0]) * indices.size());
    indexBuffer->copy((void *)indices.data(),
                      sizeof(indices[0]) * indices.size());

    std::shared_ptr<buffer::VertexBuffer> vertexBuffer =
        std::make_shared<buffer::VertexBuffer>(deviceHandler, commandBuffer,
                                               sizeof(vertices[0]) *
                                                   vertices.size());
    vertexBuffer->copy((void *)vertices.data(),
                       sizeof(vertices[0]) * vertices.size());

    renderer::Renderer<graphics_pipeline::CustomRasterPipeline> renderer =
        renderer::Renderer(window, instance->instance, surface->surface,
                           deviceHandler, swapChain, commandBuffer, pipeline,
                           descriptorSets, vertexBuffer, indexBuffer);

    while (!static_cast<bool>(glfwWindowShouldClose(window))) {
        glfwPollEvents();

        auto currentTime = std::chrono::high_resolution_clock::now();

        float time = std::chrono::duration<float, std::chrono::seconds::period>(
                         currentTime - startTime)
                         .count();

        cam->timePassed =
            std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - prevTime)
                .count();
        prevTime = currentTime;

        ubo = camera->transformMatrices(swapChain->swapChainExtent.width,
                                        swapChain->swapChainExtent.height);

        ubo.model *= glm::rotate(glm::mat4(1.0F),
                                 time * glm::radians(geometry::HALF_ROTATION),
                                 glm::vec3(0.0F, 0.0F, 1.0F));

        uniformBuffer->fastCopy((void *)&ubo, sizeof(ubo));

        renderer.drawFrame();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
