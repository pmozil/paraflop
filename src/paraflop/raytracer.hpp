#include "common.hpp"
#include "gltf_model/model.hpp"
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

        createBottomLevelAccelerationStructure();
        createTopLevelAccelerationStructure();

        uint32_t width = m_swapChain->swapChainExtent.width;
        uint32_t height = m_swapChain->swapChainExtent.height;
        createStorageImage(m_swapChain->swapChainImageFormat,
                           {width, height, 1});
        createUniformBuffer();
        createRayTracingPipeline();
        createShaderBindingTables();
        createDescriptorSets();
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
        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE,
                     VkDeviceSize offset = 0);
    } ubo;

    bool resized = false;
    GLFWwindow *window;

    VkSubmitInfo submitInfo;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

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
