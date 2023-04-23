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
    };

  private:
    GLFWwindow *window;
    VkSurfaceKHR &surface;
    device::DeviceHandler &deviceHandler;
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
};
} // namespace swap_chain
