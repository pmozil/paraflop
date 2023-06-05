#include "common.hpp"
#include "geometry/camera.hpp"
#include "geometry/uniform_buffer_objects.hpp"
#include "geometry/vertex.hpp"
#include "interface/camera_rotation.hpp"
#include "interface/custom_graphics_pipeline.hpp"
#include "interface/glfw_callbacks.hpp"
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
#include "vulkan_utils/texture.hpp"
#include "vulkan_utils/vertex_buffer.hpp"
#include "vulkan_utils/vk_instance.hpp"
#include "vulkan_utils/window.hpp"

const std::vector<geometry::Vertex> vertices = {
    {{-0.5F, -0.5F, 1.0F}, {1.0F, 0.0F, 0.0F}, {1.0F, 0.0F}},
    {{0.5F, -0.5F, 1.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 0.0F}},
    {{0.5F, 0.5F, 1.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 1.0F}},
    {{-0.5F, 0.5F, 1.0F}, {1.0F, 1.0F, 1.0F}, {1.0F, 1.0F}},

    {{-0.5F, -0.5F, -1.0F}, {1.0F, 0.0F, 0.0F}, {1.0F, 0.0F}},
    {{0.5F, -0.5F, -1.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 0.0F}},
    {{0.5F, 0.5F, -1.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 1.0F}},
    {{-0.5F, 0.5F, -1.0F}, {1.0F, 1.0F, 1.0F}, {1.0F, 1.0F}}};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

int main() {
    std::vector<const char *> validation = {"VK_LAYER_KHRONOS_validation"};

    auto *cam = new CameraRotation(nullptr);

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
    std::shared_ptr<command_buffer::ImageCommandBufferHandler> commandBuffer =
        std::make_shared<command_buffer::ImageCommandBufferHandler>(
            deviceHandler, swapChain, pipeline);

    std::shared_ptr<buffer::UniformBuffer> uniformBuffer =
        std::make_shared<buffer::UniformBuffer>(
            deviceHandler, commandBuffer, sizeof(geometry::TransformMatrices));

    std::shared_ptr<geometry::Camera> camera =
        std::make_shared<geometry::Camera>();

    cam->camera = camera;

    glm::vec3 pos = {0.0F, 0.0F, 1.0F};

    camera->position = pos;

    camera->calcRotation(geometry::HALF_ROTATION, 0.0F);

    geometry::TransformMatrices ubo = camera->transformMatrices(
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
