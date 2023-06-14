#include "common.hpp"
#include "gltf_model/model.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/raytracer_base.hpp"
#include "vulkan_utils/uniform_buffer.hpp"

class Raytracer : public raytracer::RaytracerBase {
  public:
    Raytracer(
        std::shared_ptr<device::DeviceHandler> m_deviceHandler,
        std::shared_ptr<swap_chain::DepthBufferSwapChain> m_swapChain,
        std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer,
        std::shared_ptr<gltf_model::Model> m_model, GLFWwindow *window)
        : raytracer::RaytracerBase(std::move(m_deviceHandler),
                                   std::move(m_swapChain),
                                   std::move(m_commandBuffer)),
          window(window), scene(std::move(m_model)) {

        raytracer::RaytracerBase::prepare();
        createBottomLevelAccelerationStructure();
        createTopLevelAccelerationStructure();

        uint32_t width = this->m_swapChain->swapChainExtent.width;
        uint32_t height = this->m_swapChain->swapChainExtent.height;
        createStorageImage(this->m_swapChain->swapChainImageFormat,
                           {width, height, 1});

        VkPipelineStageFlags stages =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        submitInfo = create_info::submitInfo();
        submitInfo.pWaitDstStageMask = &stages;
        submitInfo.waitSemaphoreCount = 1;
        // submitInfo.pWaitSemaphores =
        //     &this->m_swapChain->renderFinishedSemaphores.dat;
        submitInfo.signalSemaphoreCount = 1;
        // submitInfo.pSignalSemaphores = &semaphores.renderComplete;

        createUniformBuffer();
        createRayTracingPipeline();
        createShaderBindingTables();
        std::cout << "BUILT TOP LEVEL AS \n";
        createDescriptorSets();
        makeCommandBuffers();
        buildCommandBuffers();
    }

    AccelerationStructure bottomLevelAS;
    AccelerationStructure topLevelAS;

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
    struct ShaderBindingTables {
        ShaderBindingTable raygen;
        ShaderBindingTable miss;
        ShaderBindingTable hit;
    } shaderBindingTables;

    struct UniformData {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
        glm::vec4 lightPos;
        int32_t vertexSize;
    } uniformData;

    std::vector<VkCommandBuffer> drawCmdBuffers;

    struct Buffer {
        VkDevice device;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDescriptorBufferInfo descriptor;
        VkDeviceSize size = 0;
        VkDeviceSize alignment = 0;
        void *mapped = nullptr;
        /** @brief Usage flags to be filled by external source at buffer
         * creation (to query at some later point) */
        VkBufferUsageFlags usageFlags;
        /** @brief Memory property flags to be filled by external source at
         * buffer creation (to query at some later point) */
        VkMemoryPropertyFlags memoryPropertyFlags;
        VkResult map(VkDevice device, VkDeviceSize size = VK_WHOLE_SIZE,
                     VkDeviceSize offset = 0);
        void destroy() const;
    } ubo;

    bool resized = false;
    GLFWwindow *window;

    VkSubmitInfo submitInfo;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

    // gltf_model::Model scene;
    std::shared_ptr<gltf_model::Model> scene;

    void createBottomLevelAccelerationStructure();
    void createTopLevelAccelerationStructure();

    void createShaderBindingTables();
    void createDescriptorSets();
    void createRayTracingPipeline();
    void createUniformBuffer();
    void handleResize();
    void buildCommandBuffers();
    void updateUniformBuffers();

    void makeCommandBuffers();
    void clearCommandBuffers();
    void renderFrame();
    void prepareFrame();
    void submitFrame();

    uint32_t imageIdx;
};
