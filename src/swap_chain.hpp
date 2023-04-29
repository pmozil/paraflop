#include "common.hpp"
#include "device.hpp"

namespace swap_chain {
class SwapChain {
  public:
    SwapChain(GLFWwindow *window, VkSurfaceKHR &surface,
              device::DeviceHandler &deviceHandler)
        : window(window), surface(surface), deviceHandler(deviceHandler) {
        createSwapChain();
        createImageViews();
        createRenderPass();
        createFrameBuffers();
    };
    VkSwapchainKHR &getSwapChain() { return swapChain; };
    VkFormat &getSwapChainImageFormat() { return swapChainImageFormat; };
    VkExtent2D &getSwapChainExtent() { return swapChainExtent; };
    VkRenderPass &getRenderPass() { return renderPass; };
    std::vector<VkImage> &getSwapChainImages() { return swapChainImages; };
    std::vector<VkImageView> &getSwapChainImageViews() {
        return swapChainImageViews;
    };
    std::vector<VkFramebuffer> &getSwapChainFrameBuffers() {
        return swapChainFramebuffers;
    };

  private:
    GLFWwindow *window;
    VkSurfaceKHR &surface;
    device::DeviceHandler &deviceHandler;
    VkRenderPass renderPass;
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFrameBuffers();
};
} // namespace swap_chain
