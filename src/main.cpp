#include "common.hpp"
#include "geometry/uniform_buffer_object.hpp"
#include "geometry/vertex.hpp"
#include "renderer/custom_graphics_pipeline.hpp"
#include "renderer/renderer.hpp"
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

const std::vector<Vertex> vertices = {{{-0.5F, -0.5F}, {1.0F, 0.0F, 0.0F}},
                                      {{0.5F, -0.5F}, {0.0F, 1.0F, 0.0F}},
                                      {{0.5F, 0.5F}, {0.0F, 0.0F, 1.0F}},
                                      {{-0.5F, 0.5F}, {1.0F, 1.0F, 1.0F}}};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

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

    VkDescriptorSetLayoutBinding uboBinding =
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0,
            1);

    std::shared_ptr<descriptor_set::DescriptorSetLayout> layout{
        new descriptor_set::DescriptorSetLayout(deviceHandler, &uboBinding, 1)};

    // std::shared_ptr<graphics_pipeline::RasterGraphicsPipeline> pipeline{
    //     new graphics_pipeline::RasterGraphicsPipeline(swapChain,
    //                                                   deviceHandler)};
    std::shared_ptr<graphics_pipeline::CustomRasterPipeline> pipeline{
        new graphics_pipeline::CustomRasterPipeline(swapChain, deviceHandler,
                                                    &layout->layout)};
    std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer{
        new command_buffer::CommandBufferHandler(deviceHandler, swapChain,
                                                 pipeline)};

    std::shared_ptr<buffer::IndexBuffer> indexBuffer{new buffer::IndexBuffer(
        deviceHandler, commandBuffer, sizeof(uint32_t) * indices.size())};
    indexBuffer->map();
    indexBuffer->copy((void *)indices.data(),
                      sizeof(uint32_t) * indices.size());

    std::shared_ptr<buffer::VertexBuffer> vertexBuffer{new buffer::VertexBuffer(
        deviceHandler, commandBuffer, sizeof(Vertex) * vertices.size())};
    vertexBuffer->map();
    vertexBuffer->copy((void *)vertices.data(),
                       sizeof(Vertex) * vertices.size());

    std::vector<buffer::UniformBuffer> uniformBuffers(
        MAX_FRAMES_IN_FLIGHT,
        buffer::UniformBuffer(deviceHandler, commandBuffer,
                              sizeof(UniformBufferObject)));

    renderer::Renderer<graphics_pipeline::CustomRasterPipeline> renderer =
        renderer::Renderer(window, instance->instance, surface->surface,
                           deviceHandler, swapChain, commandBuffer, pipeline,
                           nullptr, vertexBuffer, indexBuffer);

    while (!static_cast<bool>(glfwWindowShouldClose(window))) {
        glfwPollEvents();
        renderer.drawFrame();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
