#include "common.hpp"
#include "custom_graphics_pipeline.hpp"
#include "geometry/camera.hpp"
#include "geometry/uniform_buffer_objects.hpp"
#include "geometry/vertex.hpp"
#include "interface/camera_rotation.hpp"
#include "interface/glfw_callbacks.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/debug.hpp"
#include "vulkan_utils/depth_buffer.hpp"
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

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

std::vector<VkWriteDescriptorSet>
getDescriptorWrites(VkDescriptorBufferInfo *bufferInfo,
                    VkDescriptorImageInfo *imageInfo);
std::vector<VkDescriptorPoolSize> getDescriptorSizes();

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
    std::shared_ptr<swap_chain::DepthBufferSwapChain> swapChain =
        std::make_shared<swap_chain::DepthBufferSwapChain>(
            window, surface->surface, deviceHandler);

    std::array<VkDescriptorSetLayoutBinding, 2> uboBindings = {
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0,
            1),
        create_info::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1),
    };

    std::shared_ptr<descriptor_set::DescriptorSetLayout> layout =
        std::make_shared<descriptor_set::DescriptorSetLayout>(
            deviceHandler, uboBindings.data(), uboBindings.size());

    std::shared_ptr<graphics_pipeline::CustomRasterPipeline> pipeline =
        std::make_shared<graphics_pipeline::CustomRasterPipeline>(
            swapChain, deviceHandler, &layout->layout);
    std::shared_ptr<command_buffer::ImageCommandBufferHandler> commandBuffer =
        std::make_shared<command_buffer::ImageCommandBufferHandler>(
            deviceHandler, swapChain, pipeline);

    std::shared_ptr<buffer::UniformBuffer> uniformBuffer =
        std::make_shared<buffer::UniformBuffer>(
            deviceHandler, commandBuffer, sizeof(geometry::TransformMatrices));

    std::shared_ptr<texture::Texture2D> texture{new texture::Texture2D(
        "textures/metal.ktx", VK_FORMAT_R8G8B8A8_UNORM, deviceHandler,
        commandBuffer, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)};

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

    auto sizes = getDescriptorSizes();
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer->buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(geometry::TransformMatrices);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.imageView = texture->view;
    imageInfo.sampler = texture->sampler;

    auto writes = getDescriptorWrites(&bufferInfo, &imageInfo);

    std::shared_ptr<descriptor_set::DescriptorSetHandler> descriptorSets =
        std::make_shared<descriptor_set::DescriptorSetHandler>(
            deviceHandler, layout, sizes, writes);

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

std::vector<VkDescriptorPoolSize> getDescriptorSizes() {
    std::vector<VkDescriptorPoolSize> sizes(2);

    sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    return sizes;
}

std::vector<VkWriteDescriptorSet>
getDescriptorWrites(VkDescriptorBufferInfo *bufferInfo,
                    VkDescriptorImageInfo *imageInfo) {
    std::vector<VkWriteDescriptorSet> descriptorWrites(2);

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = imageInfo;

    return descriptorWrites;
}
