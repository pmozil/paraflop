#include "common.hpp"
#include "gltf_model/model.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/raytracer_base.hpp"
#include "vulkan_utils/uniform_buffer.hpp"
/**
 * \class Raytracer
 * \brief A class representing a raytracer for Vulkan-based rendering.
 *
 * This class extends the `raytracer::RaytracerBase` class and provides
 * additional functionality for creating and managing a raytracer, including
 * handling acceleration structures, shader binding tables, descriptor sets, and
 * uniform buffers.
 */
class Raytracer : public raytracer::RaytracerBase {
  public:
    /**
     * \brief Constructs a Raytracer object.
     * \param m_deviceHandler A shared pointer to a DeviceHandler object.
     * \param m_swapChain A shared pointer to a DepthBufferSwapChain object.
     * \param m_commandBuffer A shared pointer to a CommandBufferHandler object.
     * \param m_model A shared pointer to a Model object representing the scene.
     * \param window A pointer to the GLFW window used for rendering.
     *
     * This constructor initializes the Raytracer object by setting up the base
     * class, preparing for rendering, creating acceleration structures, a
     * uniform buffer, storage image, lights buffer, ray tracing pipeline,
     * shader binding tables, descriptor sets, and command buffers.
     */
    Raytracer(
        std::shared_ptr<device::DeviceHandler> m_deviceHandler,
        std::shared_ptr<swap_chain::DepthBufferSwapChain> m_swapChain,
        std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer,
        std::shared_ptr<gltf_model::Model> m_model, GLFWwindow *window)
        : raytracer::RaytracerBase(std::move(m_deviceHandler),
                                   std::move(m_swapChain),
                                   std::move(m_commandBuffer)),
          window(window), scene(std::move(m_model)) {
        vkDeviceWaitIdle(*this->m_deviceHandler);

        raytracer::RaytracerBase::prepare();
        createBottomLevelAccelerationStructure();
        createTopLevelAccelerationStructure();
        createUniformBuffer();
        setupColorsBuffer(false);

        uint32_t width = this->m_swapChain->swapChainExtent.width;
        uint32_t height = this->m_swapChain->swapChainExtent.height;
        createStorageImage(this->m_swapChain->swapChainImageFormat,
                           {width, height, 1});

        VkPipelineStageFlags stages =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        submitInfo = create_info::submitInfo();
        submitInfo.pWaitDstStageMask = &stages;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.signalSemaphoreCount = 1;

        setupLightsBuffer();

        createRayTracingPipeline();
        createShaderBindingTables();
        createDescriptorSets();

        makeCommandBuffers();
        buildCommandBuffers();
    }

    /**
     * \brief Destroys the Raytracer object.
     *
     * This destructor cleans up all the resources used by the Raytracer,
     * including pipelines, pipeline layouts, descriptor set layouts, lights
     * buffer, storage image, acceleration structures, shader binding tables,
     * and the uniform buffer.
     */
    ~Raytracer() {
        vkDeviceWaitIdle(*m_deviceHandler);

        vkDestroyPipeline(*m_deviceHandler, pipeline, nullptr);
        vkDestroyPipelineLayout(*m_deviceHandler, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(*m_deviceHandler, descriptorSetLayout,
                                     nullptr);
        cleanupLightsBuffer();
        cleanupColorsBuffer();
        deleteStorageImage();
        deleteAccelerationStructure(bottomLevelAS);
        deleteAccelerationStructure(topLevelAS);
        shaderBindingTables.raygen.destroy();
        shaderBindingTables.miss.destroy();
        shaderBindingTables.hit.destroy();
        ubo.destroy(*m_deviceHandler);
    }

    /**
     * \brief The bottom-level acceleration structure.
     */
    AccelerationStructure bottomLevelAS;

    /**
     * \brief The top-level acceleration structure.
     */
    AccelerationStructure topLevelAS;

    /**
     * \brief The vector of shader groups used in the ray tracing pipeline.
     */
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};

    /**
     * \brief The shader binding tables used in ray tracing.
     */
    struct ShaderBindingTables {
        raytracer::ShaderBindingTable
            raygen; /**< The ray generation shader binding table. */
        raytracer::ShaderBindingTable
            miss; /**< The miss shader binding table. */
        raytracer::ShaderBindingTable hit; /**< The hit shader binding table. */
    } shaderBindingTables;

    /**
     * \brief The uniform data used in the raytracer.
     */
    struct UniformData {
        glm::mat4 viewInverse;   /**< The inverse of the view matrix. */
        glm::mat4 projInverse;   /**< The inverse of the projection matrix. */
        int32_t vertexSize;      /**< The vertex size. */
        int32_t lightsCount = 0; /**< The number of lights in the scene. */
        float dTime = 0.0F;
        int32_t width = 0;
    } uniformData;

    /**
     * \brief The lights buffer used in the raytracer.
     */
    struct Lights {
        std::vector<glm::vec4>
            lights;      /**< The vector of lights in the scene. */
        VkBuffer buffer; /**< The lights buffer. */
        VkDeviceMemory
            memory;        /**< The device memory associated with the buffer. */
        VkDeviceSize size; /**< The size of the buffer. */
        void *mapped;      /**< A pointer to the mapped memory of the buffer. */
        VkBufferUsageFlags usageFlags; /**< The usage flags of the buffer. */
        VkMemoryPropertyFlags memoryPropertyFlags; /**< The memory property
                                                      flags of the buffer. */
    } lights;

    /**
     * \brief The color buffer used in the raytracer.
     */
    struct ReSTIRColors {
        VkBuffer buffer; /**< The lights buffer. */
        VkDeviceMemory
            memory;        /**< The device memory associated with the buffer. */
        VkDeviceSize size; /**< The size of the buffer. */
        VkBufferUsageFlags usageFlags; /**< The usage flags of the buffer. */
        VkMemoryPropertyFlags memoryPropertyFlags; /**< The memory property
                                                      flags of the buffer. */
    } colorBuffer;

    /**
     * \brief The vector of draw command buffers.
     */
    std::vector<VkCommandBuffer> drawCmdBuffers;

    /**
     * \brief The uniform buffer object used in the raytracer.
     */
    struct Buffer {
        VkBuffer buffer; /**< The buffer object. */
        VkDeviceMemory
            memory; /**< The device memory associated with the buffer. */
        VkDescriptorBufferInfo
            descriptor;         /**< The descriptor for the buffer. */
        VkDeviceSize size;      /**< The size of the buffer. */
        VkDeviceSize alignment; /**< The alignment of the buffer. */
        void *mapped; /**< A pointer to the mapped memory of the buffer. */
        VkBufferUsageFlags usageFlags; /**< The usage flags of the buffer. */
        VkMemoryPropertyFlags memoryPropertyFlags; /**< The memory property
                                                      flags of the buffer. */

        /**
         * \brief Maps the buffer memory.
         * \param device The Vulkan device.
         * \param size The size of the memory region to map (optional).
         * \param offset The offset into the memory region to map (optional).
         * \return The result of the memory mapping operation.
         */
        VkResult map(VkDevice device, VkDeviceSize size = VK_WHOLE_SIZE,
                     VkDeviceSize offset = 0);

        /**
         * \brief Destroys the buffer object.
         */
        void destroy(VkDevice device) const;
    } ubo;

    bool resized =
        false; /**< Flag indicating whether the window has been resized. */
    GLFWwindow *window;              /**< Pointer to the GLFW window. */
    VkSubmitInfo submitInfo;         /**< The Vulkan submit info structure. */
    VkPipeline pipeline;             /**< The ray tracing pipeline. */
    VkPipelineLayout pipelineLayout; /**< The pipeline layout. */
    VkDescriptorSet descriptorSet;   /**< The descriptor set. */
    VkDescriptorSetLayout
        descriptorSetLayout; /**< The descriptor set layout. */
    VkSampler sampler;       /**< The texture sampler. */
    std::vector<VkDescriptorImageInfo>
        textureDescriptors; /**< The vector of texture descriptors. */
    std::shared_ptr<gltf_model::Model>
        scene; /**< The shared pointer to the scene model. */

    /**
     * \brief Creates the bottom-level acceleration structure.
     */
    void createBottomLevelAccelerationStructure();

    /**
     * \brief Creates the top-level acceleration structure.
     */
    void createTopLevelAccelerationStructure();

    /**
     * \brief Sets up the lights buffer.
     */
    void setupLightsBuffer();

    /**
     * \brief Sets up the colors buffer.
     */
    void setupColorsBuffer(bool setupDescr = true);

    /**
     * \brief Updates the lights buffer with new lights.
     * \param newLights The vector of new lights.
     */
    void updateLightsBuffer(std::vector<glm::vec4> newLights);

    /**
     * \brief Cleans up the lights buffer.
     */
    void cleanupLightsBuffer();

    /**
     * \brief Clean up the colors buffer.
     */
    void cleanupColorsBuffer();

    /**
     * \brief Creates the shader binding tables.
     */
    void createShaderBindingTables();

    /**
     * \brief Creates the descriptor sets.
     */
    void createDescriptorSets();

    /**
     * \brief Creates the ray tracing pipeline.
     */
    void createRayTracingPipeline();

    /**
     * \brief Creates the uniform buffer.
     */
    void createUniformBuffer();

    /**
     * \brief Handles the window resize event.
     */
    void handleResize();

    /**
     * \brief Builds the command buffers.
     */
    void buildCommandBuffers();

    /**
     * \brief Updates the uniform buffers.
     */
    void updateUniformBuffers();

    /**
     * \brief Updates the uniform buffers with the given projection and view
     * matrices.
     * \param proj The projection matrix.
     * \param view The view matrix.
     */
    void updateUniformBuffers(glm::mat4 proj, glm::mat4 view);

    /**
     * \brief Creates the command buffers.
     */
    void makeCommandBuffers();

    /**
     * \brief Clears the command buffers.
     */
    void clearCommandBuffers();

    /**
     * \brief Renders a frame.
     */
    void renderFrame();

    /**
     * \brief Prepares a frame for rendering.
     */
    void prepareFrame();

    /**
     * \brief Submits a frame for rendering.
     */
    void submitFrame();

    uint32_t imageIdx; /**< The index of the image being rendered. */
    uint32_t curFrame; /**< The current frame number. */
};
