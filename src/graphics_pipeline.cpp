#include "graphics_pipeline.hpp"
#include "create_info.hpp"

namespace graphics_pipeline {
VkShaderModule
AbstractGraphicsPipeline::createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(deviceHandler->logicalDevice, &createInfo,
                                  nullptr, &shaderModule));

    return shaderModule;
}
RasterGraphicsPipeline::RasterGraphicsPipeline(
    swap_chain::SwapChain *swapChain, device::DeviceHandler *deviceHandler)
    : AbstractGraphicsPipeline(swapChain, deviceHandler) {
    createGraphicsPipeline();
}

CustomGraphicsPipeline::CustomGraphicsPipeline(
    swap_chain::SwapChain *swapChain, device::DeviceHandler *deviceHandler,
    VkPipelineLayoutCreateInfo &pipelineLayoutCreateInfo,
    VkGraphicsPipelineCreateInfo &pipelineCreateInfo)
    : AbstractGraphicsPipeline(swapChain, deviceHandler),
      pipelineCreateInfo(pipelineCreateInfo),
      pipelineLayoutCreateInfo(pipelineLayoutCreateInfo) {
    createGraphicsPipeline();
}

void CustomGraphicsPipeline::createGraphicsPipeline() {
    VK_CHECK(vkCreatePipelineLayout(deviceHandler->logicalDevice,
                                    &pipelineLayoutCreateInfo, nullptr,
                                    &pipelineLayout));
    VK_CHECK(vkCreateGraphicsPipelines(deviceHandler->logicalDevice,
                                       VK_NULL_HANDLE, 1, &pipelineCreateInfo,
                                       nullptr, &graphicsPipeline));
}

std::vector<char>
AbstractGraphicsPipeline::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void RasterGraphicsPipeline::createGraphicsPipeline() {
    auto vertShaderCode = readFile("vert.spv");
    auto fragShaderCode = readFile("frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo =
        create_info::shaderStageInfo(vertShaderModule,
                                     VK_SHADER_STAGE_VERTEX_BIT);
    VkPipelineShaderStageCreateInfo fragShaderStageInfo =
        create_info::shaderStageInfo(fragShaderModule,
                                     VK_SHADER_STAGE_FRAGMENT_BIT);

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
        vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkExtent2D swapChainExtent = swapChain->swapChainExtent;
    VkViewport viewport = {};
    viewport.x = 0.0F;
    viewport.y = 0.0F;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0F;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0F;
    colorBlending.blendConstants[1] = 0.0F;
    colorBlending.blendConstants[2] = 0.0F;
    colorBlending.blendConstants[3] = 0.0F;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VK_CHECK(vkCreatePipelineLayout(deviceHandler->logicalDevice,
                                    &pipelineLayoutInfo, nullptr,
                                    &pipelineLayout));

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = swapChain->getRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VK_CHECK(vkCreateGraphicsPipelines(deviceHandler->logicalDevice,
                                       VK_NULL_HANDLE, 1, &pipelineInfo,
                                       nullptr, &graphicsPipeline));

    vkDestroyShaderModule(deviceHandler->logicalDevice, fragShaderModule,
                          nullptr);
    vkDestroyShaderModule(deviceHandler->logicalDevice, vertShaderModule,
                          nullptr);
}

void AbstractGraphicsPipeline::cleanup() {
    vkDestroyPipeline(deviceHandler->logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(deviceHandler->logicalDevice, pipelineLayout,
                            nullptr);
}
} // namespace graphics_pipeline
