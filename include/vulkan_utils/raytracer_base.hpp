#pragma once
#include "common.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/shader_binding_table.hpp"

namespace raytracer {
/**
 * \class RaytracerBase
 * \brief Base class for ray tracing operations in Vulkan.
 *
 * This class provides a base functionality for performing ray tracing
 * operations in Vulkan. It encapsulates various Vulkan functions and structures
 * related to ray tracing, including acceleration structures, storage images,
 * scratch buffers, and shader binding tables.
 */
class RaytracerBase {
  public:
    /**
     * \brief Constructs a RaytracerBase object.
     *
     * \param m_deviceHandler The device handler for Vulkan operations.
     * \param m_swapChain The swap chain for rendering.
     * \param m_commandBuffer The command buffer handler for Vulkan operations.
     *
     * \fn RaytracerBase::RaytracerBase(
     *      std::shared_ptr<device::DeviceHandler> m_deviceHandler,
     *      std::shared_ptr<swap_chain::DepthBufferSwapChain> m_swapChain,
     *      std::shared_ptr<command_buffer::CommandBufferHandler>
     * m_commandBuffer)
     */
    RaytracerBase(
        std::shared_ptr<device::DeviceHandler> m_deviceHandler,
        std::shared_ptr<swap_chain::DepthBufferSwapChain> m_swapChain,
        std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer)
        : m_deviceHandler(std::move(m_deviceHandler)),
          m_swapChain(std::move(m_swapChain)),
          m_commandBuffer(std::move(m_commandBuffer)) {}

    // Function pointers for ray tracing related stuff
    PFN_vkGetBufferDeviceAddressKHR
        vkGetBufferDeviceAddressKHR; /**< Function pointer for
                                        vkGetBufferDeviceAddressKHR. */
    PFN_vkCreateAccelerationStructureKHR
        vkCreateAccelerationStructureKHR; /**< Function pointer for
                                             vkCreateAccelerationStructureKHR.
                                           */
    PFN_vkDestroyAccelerationStructureKHR
        vkDestroyAccelerationStructureKHR; /**< Function pointer for
                                              vkDestroyAccelerationStructureKHR.
                                            */
    PFN_vkGetAccelerationStructureBuildSizesKHR
        vkGetAccelerationStructureBuildSizesKHR; /**< Function pointer for
                                                    vkGetAccelerationStructureBuildSizesKHR.
                                                  */
    PFN_vkGetAccelerationStructureDeviceAddressKHR
        vkGetAccelerationStructureDeviceAddressKHR; /**< Function pointer for
                                                       vkGetAccelerationStructureDeviceAddressKHR.
                                                     */
    PFN_vkBuildAccelerationStructuresKHR
        vkBuildAccelerationStructuresKHR; /**< Function pointer for
                                             vkBuildAccelerationStructuresKHR.
                                           */
    PFN_vkCmdBuildAccelerationStructuresKHR
        vkCmdBuildAccelerationStructuresKHR; /**< Function pointer for
                                                vkCmdBuildAccelerationStructuresKHR.
                                              */
    PFN_vkCmdTraceRaysKHR
        vkCmdTraceRaysKHR; /**< Function pointer for vkCmdTraceRaysKHR. */
    PFN_vkGetRayTracingShaderGroupHandlesKHR
        vkGetRayTracingShaderGroupHandlesKHR; /**< Function pointer for
                                                 vkGetRayTracingShaderGroupHandlesKHR.
                                               */
    PFN_vkCreateRayTracingPipelinesKHR
        vkCreateRayTracingPipelinesKHR; /**< Function pointer for
                                           vkCreateRayTracingPipelinesKHR. */

    // Available features and properties
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR
        rayTracingPipelineProperties{}; /**< Ray tracing pipeline properties. */
    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        accelerationStructureFeatures{}; /**< Acceleration structure features.
                                          */

    // Enabled features and properties
    VkPhysicalDeviceBufferDeviceAddressFeatures
        enabledBufferDeviceAddresFeatures{}; /**< Enabled buffer device address
                                              features. */
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR
        enabledRayTracingPipelineFeatures{}; /**< Enabled ray tracing pipeline
                                              features. */
    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        enabledAccelerationStructureFeatures{}; /**< Enabled acceleration
                                                 structure features. */

    /**
     * \struct ScratchBuffer
     * \brief Holds information for a ray tracing scratch buffer.
     */
    struct ScratchBuffer {
        uint64_t deviceAddress; /**< Device address of the scratch buffer. */
        VkBuffer handle;        /**< Handle of the scratch buffer. */
        VkDeviceMemory
            memory; /**< Device memory associated with the scratch buffer. */
    };

    /**
     * \struct AccelerationStructure
     * \brief Holds information for a ray tracing acceleration structure.
     */
    struct AccelerationStructure {
        VkAccelerationStructureKHR
            handle; /**< Handle of the acceleration structure. */
        uint64_t
            deviceAddress; /**< Device address of the acceleration structure. */
        VkDeviceMemory memory; /**< Device memory associated with the
                                  acceleration structure. */
        VkBuffer
            buffer; /**< Buffer associated with the acceleration structure. */
    };

    /**
     * \struct StorageImage
     * \brief Holds information for a storage image used for output by ray
     * tracing shaders.
     */
    struct StorageImage {
        VkDeviceMemory
            memory;    /**< Device memory associated with the storage image. */
        VkImage image; /**< Handle of the storage image. */
        VkImageView view; /**< ImageView of the storage image. */
        VkFormat format;  /**< Format of the storage image. */
    } storageImage;

    ScratchBuffer createScratchBuffer(VkDeviceSize size);
    VkDescriptorPool
        descriptorPool; /**< Descriptor pool for ray tracing operations. */
    void deleteScratchBuffer(ScratchBuffer &scratchBuffer);
    void createAccelerationStructure(
        AccelerationStructure &accelerationStructure,
        VkAccelerationStructureTypeKHR type,
        VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
    void
    deleteAccelerationStructure(AccelerationStructure &accelerationStructure);
    uint64_t getBufferDeviceAddress(VkBuffer buffer);
    void createStorageImage(VkFormat format, VkExtent3D extent);
    void deleteStorageImage();
    VkStridedDeviceAddressRegionKHR
    getSbtEntryStridedDeviceAddressRegion(VkBuffer buffer,
                                          uint32_t handleCount);
    void createShaderBindingTable(ShaderBindingTable &shaderBindingTable,
                                  uint32_t handleCount);

    VkPipelineShaderStageCreateInfo loadShader(std::string fileName,
                                               VkShaderStageFlagBits stage);
    VkPipelineShaderStageCreateInfo loadShader(std::string &fileName,
                                               VkShaderStageFlagBits stage);

    /**
     * \brief Prepares the ray tracer for execution.
     *
     * This function prepares the ray tracer by updating the render pass and
     * loading the shader modules.
     *
     * \fn void RaytracerBase::prepare()
     */
    void prepare();

    std::vector<VkShaderModule>
        shaderModules; /**< Vector of loaded shader modules. */

  protected:
    /**
     * \brief Updates the default render pass with different color attachment
     * load operations.
     *
     * This function updates the default render pass with different color
     * attachment load operations based on the format of the storage image.
     *
     * \fn void RaytracerBase::updateRenderPass()
     */
    void updateRenderPass();

    std::shared_ptr<device::DeviceHandler>
        m_deviceHandler; /**< Device handler for Vulkan operations. */
    std::shared_ptr<swap_chain::DepthBufferSwapChain>
        m_swapChain; /**< Swap chain for rendering. */
    std::shared_ptr<command_buffer::CommandBufferHandler>
        m_commandBuffer; /**< Command buffer handler for Vulkan operations. */
};
} // namespace raytracer
