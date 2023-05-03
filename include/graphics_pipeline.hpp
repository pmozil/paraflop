#pragma once
#include "common.hpp"
#include "device.hpp"
#include "swap_chain.hpp"

namespace graphics_pipeline {
using pipeline_type = enum { PARAFLOP_GRAPHICS_PIPELINE_RASTER };

class AbstractGraphicsPipeline {
  public:
    inline VkPipeline &getGraphicsPipeline() { return graphicsPipeline; }
    inline VkPipelineLayout &getGraphicsPipelineLayout() {
        return pipelineLayout;
    }

  protected:
    AbstractGraphicsPipeline(swap_chain::SwapChain *swapChain,
                             device::DeviceHandler *deviceHandler)
        : swapChain(swapChain), deviceHandler(deviceHandler){};
    static std::vector<char> readFile(const std::string &filename);
    swap_chain::SwapChain *swapChain;
    device::DeviceHandler *deviceHandler;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkShaderModule createShaderModule(const std::vector<char> &code);
    virtual void createGraphicsPipeline() = 0;
};

class CustomGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    CustomGraphicsPipeline(swap_chain::SwapChain *swapChain,
                           device::DeviceHandler *deviceHandler,
                           VkPipelineLayoutCreateInfo &pipelineLayoutCreateInfo,
                           VkGraphicsPipelineCreateInfo &pipelineCreateInfo);
    ~CustomGraphicsPipeline();

  private:
    void createGraphicsPipeline() override;
    VkGraphicsPipelineCreateInfo &pipelineCreateInfo;
    VkPipelineLayoutCreateInfo &pipelineLayoutCreateInfo;
};

class RasterGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    RasterGraphicsPipeline(swap_chain::SwapChain *swapChain,
                           device::DeviceHandler *deviceHandler);
    ~RasterGraphicsPipeline();

  private:
    void createGraphicsPipeline() override;
};
} // namespace graphics_pipeline
