#pragma once
#include "common.hpp"
#include "gltf_model/gltf_common.hpp"
#include "gltf_model/model.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/graphics_pipeline.hpp"
#include "vulkan_utils/swap_chain.hpp"

namespace graphics_pipeline {
class CustomRasterPipeline : public AbstractGraphicsPipeline {
  private:
    std::shared_ptr<gltf_model::Model> m_model;

  public:
    /**
     * \brief Constructs a CustomRasterPipeline object.
     *
     * \param m_swapChain The shared pointer to the SwapChain object.
     * \param m_deviceHandler The shared pointer to the DeviceHandler object.
     * \param layout The pointer to the Vulkan descriptor set layout.
     */
    CustomRasterPipeline(std::shared_ptr<swap_chain::SwapChain> m_swapChain,
                         std::shared_ptr<device::DeviceHandler> m_deviceHandler,
                         std::shared_ptr<gltf_model::Model> m_model,
                         VkDescriptorSetLayout *layout)
        : AbstractGraphicsPipeline(std::move(m_swapChain),
                                   std::move(m_deviceHandler), layout),
          m_model(std::move(m_model)) {
        createGraphicsPipeline();
    }

    /**
     * \brief Creates the custom raster graphics pipeline.
     */
    void createGraphicsPipeline() override {
        std::array<VkDescriptorSetLayout, 2> setLayouts = {
            gltf_model::descriptorSetLayoutUbo,
            gltf_model::descriptorSetLayoutImage};

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = setLayouts.size();
        pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(glm::mat4);

        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
        VK_CHECK(vkCreatePipelineLayout(*m_deviceHandler,
                                        &pipelineLayoutCreateInfo, nullptr,
                                        &pipelineLayout));

        const VkVertexInputBindingDescription vertexInputBinding =
            create_info::vertexInputBindingDescription(
                0, sizeof(gltf_model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

        const std::vector<VkVertexInputAttributeDescription>
            vertexInputAttributes = {
                create_info::vertexInputAttributeDescription(
                    0, 0, VK_FORMAT_R32G32B32_SFLOAT,
                    offsetof(gltf_model::Vertex, pos)),
                create_info::vertexInputAttributeDescription(
                    0, 1, VK_FORMAT_R32G32B32_SFLOAT,
                    offsetof(gltf_model::Vertex, normal)),
                create_info::vertexInputAttributeDescription(
                    0, 2, VK_FORMAT_R32G32B32_SFLOAT,
                    offsetof(gltf_model::Vertex, uv)),
                create_info::vertexInputAttributeDescription(
                    0, 3, VK_FORMAT_R32G32B32_SFLOAT,
                    offsetof(gltf_model::Vertex, color)),
                create_info::vertexInputAttributeDescription(
                    0, 4, VK_FORMAT_R32G32B32_SFLOAT,
                    offsetof(gltf_model::Vertex, tangent)),
            };

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexBindingDescriptions =
            &vertexInputBinding;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInputStateCreateInfo.pVertexAttributeDescriptions =
            vertexInputAttributes.data();

        VkPipelineInputAssemblyStateCreateInfo
            pipelineInputAssemblyStateCreateInfo{};
        pipelineInputAssemblyStateCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipelineInputAssemblyStateCreateInfo.topology =
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineInputAssemblyStateCreateInfo.flags = 0;
        pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineRasterizationStateCreateInfo
            pipelineRasterizationStateCreateInfo{};
        pipelineRasterizationStateCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        pipelineRasterizationStateCreateInfo.frontFace =
            VK_FRONT_FACE_COUNTER_CLOCKWISE;
        pipelineRasterizationStateCreateInfo.flags = 0;
        pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        pipelineRasterizationStateCreateInfo.lineWidth = 1.0F;

        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
        pipelineColorBlendAttachmentState.colorWriteMask = 0xF;
        pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
        pipelineColorBlendStateCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipelineColorBlendStateCreateInfo.attachmentCount = 1;
        pipelineColorBlendStateCreateInfo.pAttachments =
            &pipelineColorBlendAttachmentState;

        VkPipelineDepthStencilStateCreateInfo
            pipelineDepthStencilStateCreateInfo{};
        pipelineDepthStencilStateCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        pipelineDepthStencilStateCreateInfo.depthCompareOp =
            VK_COMPARE_OP_LESS_OR_EQUAL;
        pipelineDepthStencilStateCreateInfo.back.compareOp =
            VK_COMPARE_OP_ALWAYS;

        VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
        pipelineViewportStateCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipelineViewportStateCreateInfo.viewportCount = 1;
        pipelineViewportStateCreateInfo.scissorCount = 1;
        pipelineViewportStateCreateInfo.flags = 0;

        VkPipelineMultisampleStateCreateInfo
            pipelineMultisampleStateCreateInfo{};
        pipelineMultisampleStateCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineMultisampleStateCreateInfo.rasterizationSamples =
            VK_SAMPLE_COUNT_1_BIT;
        pipelineMultisampleStateCreateInfo.flags = 0;

        const std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
        pipelineDynamicStateCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
        pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
        pipelineDynamicStateCreateInfo.flags = 0;

        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        // Create an array of shader stages
        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
            vertShaderStageInfo, fragShaderStageInfo};

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType =
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.layout = pipelineLayout;
        pipelineCreateInfo.renderPass = m_swapChain->getRenderPass();
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.flags = 0;
        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

        pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineCreateInfo.pInputAssemblyState =
            &pipelineInputAssemblyStateCreateInfo;
        pipelineCreateInfo.pRasterizationState =
            &pipelineRasterizationStateCreateInfo;
        pipelineCreateInfo.pColorBlendState =
            &pipelineColorBlendStateCreateInfo;
        pipelineCreateInfo.pMultisampleState =
            &pipelineMultisampleStateCreateInfo;
        pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
        pipelineCreateInfo.pDepthStencilState =
            &pipelineDepthStencilStateCreateInfo;
        pipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;

        pipelineCreateInfo.stageCount =
            static_cast<uint32_t>(shaderStages.size());
        pipelineCreateInfo.pStages = shaderStages.data();
    }
};

} // namespace graphics_pipeline
