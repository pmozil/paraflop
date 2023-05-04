#include "command_buffer.hpp"
#include "common.hpp"
#include "debug.hpp"
#include "device.hpp"
#include "graphics_pipeline.hpp"
#include "renderer.hpp"
#include "surface.hpp"
#include "swap_chain.hpp"
#include "vk_instance.hpp"
#include "window.hpp"

int main() {
    std::vector<const char *> validation = {"VK_LAYER_KHRONOS_validation"};

    std::vector<const char *> devExt = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    GLFWwindow *window = window::initWindow(nullptr, nullptr);
    VkInstance instance = vk_instance::createDefaultVkInstance(nullptr);
    debug::createDebugMessenger(instance);
    VkSurfaceKHR surface = surface::createSurface(instance, window, nullptr);
    std::cout << "DEBUG: ";
    std::cout << "Creating device handler"
              << "\n";
    device::DeviceHandler deviceHandler =
        device::DeviceHandler(devExt, validation, instance, surface);
    std::cout << "DEBUG: ";
    std::cout << "Creating swap chain"
              << "\n";
    swap_chain::SwapChain swapChain =
        swap_chain::SwapChain(window, surface, &deviceHandler);
    std::cout << "DEBUG: ";
    std::cout << "Creating pipeline"
              << "\n";
    graphics_pipeline::RasterGraphicsPipeline pipeline =
        graphics_pipeline::RasterGraphicsPipeline(&swapChain, &deviceHandler);
    std::cout << "DEBUG: ";
    std::cout << "Creating command buffer"
              << "\n";
    command_buffer::CommandBufferHandler commandBuffer =
        command_buffer::CommandBufferHandler(&deviceHandler, &swapChain,
                                             &pipeline);

    renderer::Renderer renderer =
        renderer::Renderer(window, instance, surface, &deviceHandler,
                           &swapChain, &commandBuffer, &pipeline);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        renderer.drawFrame();
    }

    return 0;
}
