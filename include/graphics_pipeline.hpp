#pragma once
#include "common.hpp"
#include "device.hpp"
#include "swap_chain.hpp"

namespace graphics_pipeline {
class AbstractGraphicsPipeline {
  public:
    inline VkPipeline &getGraphicsPipeline() { return graphicsPipeline; }
    inline VkPipelineLayout &getGraphicsPipelineLayout() {
        return pipelineLayout;
    }
    virtual void createGraphicsPipeline() = 0;
    void cleanup();

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
};

class CustomGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    CustomGraphicsPipeline(swap_chain::SwapChain *swapChain,
                           device::DeviceHandler *deviceHandler,
                           VkPipelineLayoutCreateInfo &pipelineLayoutCreateInfo,
                           VkGraphicsPipelineCreateInfo &pipelineCreateInfo);
    ~CustomGraphicsPipeline();
    void createGraphicsPipeline() override;

  private:
    VkGraphicsPipelineCreateInfo &pipelineCreateInfo;
    VkPipelineLayoutCreateInfo &pipelineLayoutCreateInfo;
};

class RasterGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    RasterGraphicsPipeline(swap_chain::SwapChain *swapChain,
                           device::DeviceHandler *deviceHandler);
    ~RasterGraphicsPipeline();
    void createGraphicsPipeline() override;
};
} // namespace graphics_pipeline
