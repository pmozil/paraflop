#pragma once
#include "vulkan_utils/common.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/swap_chain.hpp"

namespace graphics_pipeline {
class AbstractGraphicsPipeline {
  public:
    virtual void createGraphicsPipeline() = 0;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    void cleanup();

  protected:
    AbstractGraphicsPipeline(swap_chain::SwapChain *m_swapChain,
                             device::DeviceHandler *m_deviceHandler)
        : m_swapChain(m_swapChain), m_deviceHandler(m_deviceHandler){};
    static std::vector<char> readFile(const std::string &filename);
    swap_chain::SwapChain *m_swapChain;
    device::DeviceHandler *m_deviceHandler;
    VkShaderModule createShaderModule(const std::vector<char> &code);
};

class CustomGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    CustomGraphicsPipeline(swap_chain::SwapChain *m_swapChain,
                           device::DeviceHandler *m_deviceHandler,
                           VkPipelineLayoutCreateInfo &pipelineLayoutCreateInfo,
                           VkGraphicsPipelineCreateInfo &pipelineCreateInfo);
    void createGraphicsPipeline() override;

  private:
    VkGraphicsPipelineCreateInfo &pipelineCreateInfo;
    VkPipelineLayoutCreateInfo &pipelineLayoutCreateInfo;
};

class RasterGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    RasterGraphicsPipeline(swap_chain::SwapChain *m_swapChain,
                           device::DeviceHandler *m_deviceHandler);
    void createGraphicsPipeline() override;
};
} // namespace graphics_pipeline
