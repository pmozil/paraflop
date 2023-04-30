#pragma once
#include "common.hpp"
#include "device.hpp"
#include "swap_chain.hpp"

namespace graphics_pipeline {
class AbstractGraphicsPipeline {
  public:
    VkPipeline &getGraphicsPipeline() { return graphicsPipeline; };
    VkPipelineLayout &getGraphicsPipelineLayout() { return pipelineLayout; };

  protected:
    AbstractGraphicsPipeline(swap_chain::SwapChain &swapChain,
                             device::DeviceHandler &deviceHandler)
        : swapChain(swapChain), deviceHandler(deviceHandler){};
    std::vector<char> readFile(const std::string &filename);
    swap_chain::SwapChain &swapChain;
    device::DeviceHandler &deviceHandler;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    virtual void createGraphicsPipeline() = 0;
    VkShaderModule createShaderModule(const std::vector<char> &code);
};

class RasterGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    RasterGraphicsPipeline(swap_chain::SwapChain &swapChain,
                           device::DeviceHandler &deviceHandler)
        : AbstractGraphicsPipeline(swapChain, deviceHandler) {
        createGraphicsPipeline();
    };

  private:
    void createGraphicsPipeline();
};
} // namespace graphics_pipeline
