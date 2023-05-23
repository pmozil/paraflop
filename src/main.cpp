#include "common.hpp"
#include "geometry/uniform_buffer_object.hpp"
#include "geometry/vertex.hpp"
#include "renderer/renderer.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/debug.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/graphics_pipeline.hpp"
#include "vulkan_utils/index_buffer.hpp"
#include "vulkan_utils/surface.hpp"
#include "vulkan_utils/swap_chain.hpp"
#include "vulkan_utils/vertex_buffer.hpp"
#include "vulkan_utils/vk_instance.hpp"
#include "vulkan_utils/window.hpp"

// graphics_pipeline::CustomGraphicsPipeline makePipeline();

int main() {
    std::vector<const char *> validation = {"VK_LAYER_KHRONOS_validation"};

    std::vector<const char *> devExt = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    GLFWwindow *window = window::initWindow(nullptr, nullptr);
    std::unique_ptr<vk_instance::Instance> instance{
        new vk_instance::Instance()};
    debug::createDebugMessenger(instance->instance);
    std::unique_ptr<surface::Surface> surface{
        new surface::Surface(instance->instance, window, nullptr)};
    std::shared_ptr<device::DeviceHandler> deviceHandler{
        new device::DeviceHandler(devExt, validation, instance->instance,
                                  surface->surface)};
    std::shared_ptr<swap_chain::SwapChain> swapChain{
        new swap_chain::SwapChain(window, surface->surface, deviceHandler)};
    std::shared_ptr<graphics_pipeline::RasterGraphicsPipeline> pipeline{
        new graphics_pipeline::RasterGraphicsPipeline(swapChain,
                                                      deviceHandler)};
    std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer{
        new command_buffer::CommandBufferHandler(deviceHandler, swapChain,
                                                 pipeline)};

    renderer::Renderer<graphics_pipeline::RasterGraphicsPipeline> renderer =
        renderer::Renderer(window, instance->instance, surface->surface,
                           deviceHandler, swapChain, commandBuffer, pipeline);

    while (!static_cast<bool>(glfwWindowShouldClose(window))) {
        glfwPollEvents();
        renderer.drawFrame();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
