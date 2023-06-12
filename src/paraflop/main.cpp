#include "common.hpp"
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

#include "custom_graphics_pipeline.hpp"

std::vector<VkWriteDescriptorSet>
getDescriptorWrites(VkDescriptorBufferInfo *bufferInfo,
                    VkDescriptorImageInfo *imageInfo);
std::vector<VkDescriptorPoolSize> getDescriptorSizes();

int main() {
    std::vector<const char *> validation = {"VK_LAYER_KHRONOS_validation"};

    std::vector<const char *> devExt = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    GLFWwindow *window =
        window::initWindow(nullptr, nullptr, nullptr, nullptr, nullptr);
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

    std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer =
        std::make_shared<command_buffer::CommandBufferHandler>(deviceHandler,
                                                               swapChain);

    std::shared_ptr<gltf_model::Model> model =
        std::make_shared<gltf_model::Model>();
    model->loadFromFile("assets/models/sponza/sponza.gltf", deviceHandler,
                        commandBuffer, deviceHandler->getTransferQueue());

    std::shared_ptr<graphics_pipeline::CustomRasterPipeline> pipeline =
        std::make_shared<graphics_pipeline::CustomRasterPipeline>(
            deviceHandler, swapChain, model);

    std::shared_ptr<command_buffer::ImageCommandBufferHandler>
        imageCommandBuffer =
            std::make_shared<command_buffer::ImageCommandBufferHandler>(
                deviceHandler, swapChain, pipeline);

    return 0;
}
