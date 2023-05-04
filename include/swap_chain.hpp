#pragma once
#include "common.hpp"
#include "device.hpp"

namespace swap_chain {
class SwapChain {
  public:
    SwapChain(GLFWwindow *window, VkSurfaceKHR &surface,
              device::DeviceHandler *deviceHandler);
    VkSwapchainKHR getSwapChain() { return swapChain; };
    VkFormat getSwapChainImageFormat() { return swapChainImageFormat; };
    VkExtent2D getSwapChainExtent() { return swapChainExtent; };
    VkRenderPass getRenderPass() { return renderPasses[0]; };
    VkRenderPass getRenderPass(int n) { return renderPasses[n]; };
    int renderPassesLength() { return renderPasses.size(); };
    std::vector<VkRenderPass> &getRenderPasses() { return renderPasses; };
    std::vector<VkImage> &getSwapChainImages() { return swapChainImages; };
    std::vector<VkImageView> &getSwapChainImageViews() {
        return swapChainImageViews;
    };
    std::vector<VkFramebuffer> &getSwapChainFrameBuffers() {
        return swapChainFramebuffers;
    };
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
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkRenderPass> renderPasses;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
};
} // namespace swap_chain
