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
  AbstractGraphicsPipeline(
      std::shared_ptr<swap_chain::SwapChain> m_swapChain,
      std::shared_ptr<device::DeviceHandler> m_deviceHandler)
      : m_swapChain(std::move(m_swapChain)),
        m_deviceHandler(std::move(m_deviceHandler)){};
  std::shared_ptr<swap_chain::SwapChain> m_swapChain;
  std::shared_ptr<device::DeviceHandler> m_deviceHandler;
  static std::vector<char> readFile(const std::string &filename);
  VkShaderModule createShaderModule(const std::vector<char> &code);
};

class CustomGraphicsPipeline : public AbstractGraphicsPipeline {
public:
  CustomGraphicsPipeline(std::shared_ptr<swap_chain::SwapChain> m_swapChain,
                         std::shared_ptr<device::DeviceHandler> m_deviceHandler,
                         VkPipelineLayoutCreateInfo &pipelineLayoutCreateInfo,
                         VkGraphicsPipelineCreateInfo &pipelineCreateInfo);
  void createGraphicsPipeline() override;

private:
  VkGraphicsPipelineCreateInfo &pipelineCreateInfo;
  VkPipelineLayoutCreateInfo &pipelineLayoutCreateInfo;
};

class RasterGraphicsPipeline : public AbstractGraphicsPipeline {
public:
  RasterGraphicsPipeline(
      std::shared_ptr<swap_chain::SwapChain> m_swapChain,
      std::shared_ptr<device::DeviceHandler> m_deviceHandler);
  void createGraphicsPipeline() override;
};
} // namespace graphics_pipeline
