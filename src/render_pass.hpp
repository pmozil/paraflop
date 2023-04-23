#include "common.hpp"
#include "device.cpp"
#include "swap_chain.hpp"

namespace render_pass {
class RenderPass {
  public:
    RenderPass(swap_chain::SwapChain &swapChain,
               device::DeviceHandler &deviceHandler)
        : swapChain(swapChain), deviceHandler(deviceHandler) {
        createRenderPass();
    };

  private:
    swap_chain::SwapChain &swapChain;
    device::DeviceHandler &deviceHandler;
    VkRenderPass renderPass;
    void createRenderPass();
};
} // namespace render_pass
