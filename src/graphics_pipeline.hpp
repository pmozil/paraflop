#pragma once
#include "common.hpp"
#include "device.hpp"
#include "swap_chain.hpp"

namespace graphics_pipeline {
class GraphicsPipelineProtocol {
  public:
    GraphicsPipelineProtocol(swap_chain::SwapChain &swapChain,
                             device::DeviceHandler &deviceHandler)
        : swapChain(swapChain), deviceHandler(deviceHandler) {
        createGraphicsPipeline();
    };

  protected:
    std::vector<char> readFile(const std::string &filename);
    swap_chain::SwapChain &swapChain;
    device::DeviceHandler &deviceHandler;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    virtual void createGraphicsPipeline(){};
    VkShaderModule createShaderModule(const std::vector<char> &code);
};

class RasterGraphicsPipeline : GraphicsPipelineProtocol {
  private:
    void createGraphicsPipeline() override;
};
} // namespace graphics_pipeline
