#include "vulkan_utils/graphics_pipeline.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/staging_buffer.hpp"
#include "vulkan_utils/vertex_buffer.hpp"

namespace graphics_pipeline {
VkShaderModule
AbstractGraphicsPipeline::createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(*m_deviceHandler, &createInfo, nullptr,
                                  &shaderModule));

    return shaderModule;
}

CustomGraphicsPipeline::CustomGraphicsPipeline(
    std::shared_ptr<swap_chain::SwapChain> m_swapChain,
    std::shared_ptr<device::DeviceHandler> m_deviceHandler)
    : AbstractGraphicsPipeline(std::move(m_swapChain),
                               std::move(m_deviceHandler), nullptr) {}

void CustomGraphicsPipeline::createGraphicsPipelineFromInfo(
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo,
    VkGraphicsPipelineCreateInfo pipelineCreateInfo) {
    VK_CHECK(vkCreatePipelineLayout(*m_deviceHandler, &pipelineLayoutCreateInfo,
                                    nullptr, &pipelineLayout));
    VK_CHECK(vkCreateGraphicsPipelines(*m_deviceHandler, VK_NULL_HANDLE, 1,
                                       &pipelineCreateInfo, nullptr,
                                       &graphicsPipeline));
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

void AbstractGraphicsPipeline::cleanup() {
    vkDestroyPipeline(m_deviceHandler->logicalDevice, graphicsPipeline,
                      nullptr);
    vkDestroyPipelineLayout(m_deviceHandler->logicalDevice, pipelineLayout,
                            nullptr);
}
} // namespace graphics_pipeline
