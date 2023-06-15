#pragma once
#include "common.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/shader_binding_table.hpp"

namespace raytracer {
class RaytracerBase {
  public:
    RaytracerBase(
        std::shared_ptr<device::DeviceHandler> m_deviceHandler,
        std::shared_ptr<swap_chain::DepthBufferSwapChain> m_swapChain,
        std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer)
        : m_deviceHandler(std::move(m_deviceHandler)),
          m_swapChain(std::move(m_swapChain)),
          m_commandBuffer(std::move(m_commandBuffer)) {}
    // Function pointers for ray tracing related stuff
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
    PFN_vkGetAccelerationStructureBuildSizesKHR
        vkGetAccelerationStructureBuildSizesKHR;
    PFN_vkGetAccelerationStructureDeviceAddressKHR
        vkGetAccelerationStructureDeviceAddressKHR;
    PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
    PFN_vkGetRayTracingShaderGroupHandlesKHR
        vkGetRayTracingShaderGroupHandlesKHR;
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

    // Available features and properties
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR
        rayTracingPipelineProperties{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        accelerationStructureFeatures{};

    // Enabled features and properties
    VkPhysicalDeviceBufferDeviceAddressFeatures
        enabledBufferDeviceAddresFeatures{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR
        enabledRayTracingPipelineFeatures{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        enabledAccelerationStructureFeatures{};

    // Holds information for a ray tracing scratch buffer that is used as a
    // temporary storage
    struct ScratchBuffer {
        uint64_t deviceAddress = 0;
        VkBuffer handle = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
    };

    // Holds information for a ray tracing acceleration structure
    struct AccelerationStructure {
        VkAccelerationStructureKHR handle;
        uint64_t deviceAddress = 0;
        VkDeviceMemory memory;
        VkBuffer buffer;
    };

    // Holds information for a storage image that the ray tracing shaders output
    // to
    struct StorageImage {
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkFormat format;
    } storageImage;

    ScratchBuffer createScratchBuffer(VkDeviceSize size);
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
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

    void prepare();
    std::vector<VkShaderModule> shaderModules;

  protected:
    // Update the default render pass with different color attachment load
    // ops
    void updateRenderPass();
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    std::shared_ptr<swap_chain::DepthBufferSwapChain> m_swapChain;
    std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer;
};
} // namespace raytracer
