#pragma once
#include "common.hpp"
#include "device.hpp"

namespace swap_chain {
class SwapChain {
  public:
    SwapChain(GLFWwindow *window, VkSurfaceKHR &surface,
              device::DeviceHandler *deviceHandler);
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkRenderPass> renderPasses;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    inline VkRenderPass getRenderPass() { return renderPasses[0]; };
    inline VkRenderPass getRenderPass(int n) { return renderPasses[n]; };
    inline int renderPassesLength() { return renderPasses.size(); };
    int createRenderPass(VkRenderPassCreateInfo renderPassInfo);
    void cleanup();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFrameBuffers();

  private:
    GLFWwindow *window;
    VkSurfaceKHR &surface;
    device::DeviceHandler *deviceHandler;

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
};
} // namespace swap_chain
