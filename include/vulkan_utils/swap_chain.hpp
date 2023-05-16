#pragma once
#include "common.hpp"
#include "vulkan_utils/device.hpp"

namespace swap_chain {
class SwapChain {
public:
  SwapChain(GLFWwindow *m_window, VkSurfaceKHR m_surface,
            std::shared_ptr<device::DeviceHandler> m_deviceHandler);
  ~SwapChain() { cleanup(); }
  VkSwapchainKHR swapChain;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkRenderPass> renderPasses;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  inline VkRenderPass getRenderPass() { return renderPasses[0]; };
  inline VkRenderPass getRenderPass(int n) { return renderPasses[n]; };
  [[nodiscard]] inline int renderPassesLength() const {
    return renderPasses.size();
  };
  int createRenderPass(VkRenderPassCreateInfo renderPassInfo);
  void cleanup();
  void createSwapChain();
  void createImageViews();
  void createRenderPass();
  void createFrameBuffers();

private:
  GLFWwindow *m_window;
  VkSurfaceKHR m_surface;
  std::shared_ptr<device::DeviceHandler> m_deviceHandler;

  static VkSurfaceFormatKHR m_chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  static VkPresentModeKHR m_chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D m_chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
};
} // namespace swap_chain
