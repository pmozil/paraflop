#pragma once
#include "common.hpp"
#include "geometry/vertex.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/swap_chain.hpp"

namespace graphics_pipeline {
class AbstractGraphicsPipeline {
  public:
    virtual void createGraphicsPipeline() = 0;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    void cleanup();
    static std::vector<char> readFile(const std::string &filename);
    VkShaderModule createShaderModule(const std::vector<char> &code);

  protected:
    AbstractGraphicsPipeline(
        std::shared_ptr<swap_chain::SwapChain> m_swapChain,
        std::shared_ptr<device::DeviceHandler> m_deviceHandler,
        VkDescriptorSetLayout *layout)
        : m_descriptorSetLayout(layout), m_swapChain(std::move(m_swapChain)),
          m_deviceHandler(std::move(m_deviceHandler)){};
    VkDescriptorSetLayout *m_descriptorSetLayout;
    std::shared_ptr<swap_chain::SwapChain> m_swapChain;
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;
    ~AbstractGraphicsPipeline() { cleanup(); }
};

class CustomGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    CustomGraphicsPipeline(
        std::shared_ptr<swap_chain::SwapChain> m_swapChain,
        std::shared_ptr<device::DeviceHandler> m_deviceHandler);

    void createGraphicsPipelineFromInfo(
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo,
        VkGraphicsPipelineCreateInfo pipelineCreateInfo);
    ~CustomGraphicsPipeline() { cleanup(); }

  private:
    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
};

class RasterGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    RasterGraphicsPipeline(
        std::shared_ptr<swap_chain::SwapChain> m_swapChain,
        std::shared_ptr<device::DeviceHandler> m_deviceHandler);
    void createGraphicsPipeline() override;
    ~RasterGraphicsPipeline() { cleanup(); }
};
} // namespace graphics_pipeline
