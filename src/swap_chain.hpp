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
    inline VkSwapchainKHR &getSwapChain() { return swapChain; };
    inline VkFormat &getSwapChainImageFormat() { return swapChainImageFormat; };
    inline VkExtent2D &getSwapChainExtent() { return swapChainExtent; };
    inline VkRenderPass &getRenderPass() { return renderPasses[0]; };
    inline VkRenderPass &getNthRenderPass(int n) { return renderPasses[n]; };
    inline int renderPassesLength() { return renderPasses.size(); };
    inline std::vector<VkRenderPass> &getRenderPasses() {
        return renderPasses;
    };
    inline std::vector<VkImage> &getSwapChainImages() {
        return swapChainImages;
    };
    inline std::vector<VkImageView> &getSwapChainImageViews() {
        return swapChainImageViews;
    };
    inline std::vector<VkFramebuffer> &getSwapChainFrameBuffers() {
        return swapChainFramebuffers;
    };
    int createRenderPass(VkRenderPassCreateInfo renderPassInfo);
    void cleanup();

  private:
    GLFWwindow *window;
    VkSurfaceKHR &surface;
    device::DeviceHandler &deviceHandler;
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkRenderPass> renderPasses;
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
