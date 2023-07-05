#include "common.hpp"
#include "gltf_model/model.hpp"
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
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
    indexingFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    indexingFeatures.pNext = nullptr;

    enabledBufferDeviceAddresFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    enabledBufferDeviceAddresFeatures.bufferDeviceAddress = VK_TRUE;
    enabledBufferDeviceAddresFeatures.pNext = &indexingFeatures;

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

    auto *cam = new CameraRotation(nullptr);

    GLFWwindow *window = window::initWindow(nullptr, handleKeyPress,
                                            handleCursor, handleFocus, cam);

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
    // model->loadFromFile("assets/models/FlightHelmet/glTF/FlightHelmet.gltf",
    //                     deviceHandler, commandBuffer,
    //                     deviceHandler->getTransferQueue(), glTFLoadingFlags);
    // model->loadFromFile("assets/models/CesiumMan/glTF/CesiumMan.gltf",
    //                     deviceHandler, commandBuffer,
    //                     deviceHandler->getTransferQueue(), glTFLoadingFlags);
    // model->loadFromFile("assets/models/retroufo_glow.gltf", deviceHandler,
    //                     commandBuffer, deviceHandler->getTransferQueue(),
    //                     glTFLoadingFlags);
    // model->loadFromFile("assets/models/vulkanscene_shadow.gltf",
    // deviceHandler,
    //                     commandBuffer, deviceHandler->getTransferQueue(),
    //                     glTFLoadingFlags);

    auto renderer =
        Raytracer(deviceHandler, swapChain, commandBuffer, model, window);

    std::shared_ptr<geometry::Camera> camera =
        std::make_shared<geometry::Camera>();

    cam->camera = camera;

    const glm::vec3 pos = {0.0F, 3.0F, -10.0F};
    camera->moveForward(0.0F);

    camera->position = pos;
    camera->position.y = -camera->position.y;

    auto mats = camera->transformMatrices(swapChain->swapChainExtent.width,
                                          swapChain->swapChainExtent.height);

    camera->position.y = -camera->position.y;

    const std::vector<glm::vec4> lightPos = {
        glm::vec4(40.0F, -50.0F, 25.0F, 10.0F),
        glm::vec4(40.0F, -50.0F, -25.0F, 6.0F)};

    renderer.updateUniformBuffers(mats.proj, mats.view);
    renderer.updateLightsBuffer(lightPos);

    static auto startTime = std::chrono::system_clock::now();
    auto prevTime = startTime;

    while (!static_cast<bool>(glfwWindowShouldClose(window))) {
        glfwPollEvents();

        auto currentTime = std::chrono::high_resolution_clock::now();

        cam->timePassed =
            std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - prevTime)
                .count();

        camera->position.y = -camera->position.y;
        mats = camera->transformMatrices(swapChain->swapChainExtent.width,
                                         swapChain->swapChainExtent.height);
        camera->position.y = -camera->position.y;

        prevTime = currentTime;

        renderer.updateUniformBuffers(mats.proj, mats.view);

        renderer.renderFrame();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
