#include "common.hpp"
#include "gltf_model/model.hpp"
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

#include "raytracer.hpp"

int main() {
    std::vector<const char *> validation = {"VK_LAYER_KHRONOS_validation"};

    std::vector<const char *> devExt = {
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkPhysicalDeviceBufferDeviceAddressFeatures
        enabledBufferDeviceAddresFeatures{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR
        enabledRayTracingPipelineFeatures{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        enabledAccelerationStructureFeatures{};
    enabledBufferDeviceAddresFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    enabledBufferDeviceAddresFeatures.bufferDeviceAddress = VK_TRUE;

    enabledRayTracingPipelineFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    enabledRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
    enabledRayTracingPipelineFeatures.pNext =
        &enabledBufferDeviceAddresFeatures;

    enabledAccelerationStructureFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    enabledAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
    enabledAccelerationStructureFeatures.pNext =
        &enabledRayTracingPipelineFeatures;

    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
    physicalDeviceFeatures2.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.pNext = &enabledAccelerationStructureFeatures;

    GLFWwindow *window =
        window::initWindow(nullptr, nullptr, nullptr, nullptr, nullptr);
    std::unique_ptr<vk_instance::Instance> instance =
        std::make_unique<vk_instance::Instance>();
    debug::createDebugMessenger(instance->instance);

    std::unique_ptr<surface::Surface> surface =
        std::make_unique<surface::Surface>(instance->instance, window, nullptr);

    std::shared_ptr<device::DeviceHandler> deviceHandler =
        std::make_shared<device::DeviceHandler>(
            devExt, validation, instance->instance, surface->surface,
            &physicalDeviceFeatures2);

    std::shared_ptr<swap_chain::DepthBufferSwapChain> swapChain =
        std::make_shared<swap_chain::DepthBufferSwapChain>(
            window, surface->surface, deviceHandler);

    std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer =
        std::make_shared<command_buffer::CommandBufferHandler>(deviceHandler,
                                                               swapChain);

    const uint32_t glTFLoadingFlags =
        gltf_model::FileLoadingFlags::PreTransformVertices |
        gltf_model::FileLoadingFlags::PreMultiplyVertexColors |
        gltf_model::FileLoadingFlags::FlipY;

    std::shared_ptr<gltf_model::Model> model =
        std::make_shared<gltf_model::Model>();
    model->loadFromFile("assets/models/sponza/sponza.gltf", deviceHandler,
                        commandBuffer, deviceHandler->getTransferQueue(),
                        glTFLoadingFlags);

    std::cout << "FINISHED LOADING MODEL\n";

    auto renderer =
        Raytracer(deviceHandler, swapChain, commandBuffer, model, window);

    while (!static_cast<bool>(glfwWindowShouldClose(window))) {
        glfwPollEvents();
        renderer.renderFrame();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
