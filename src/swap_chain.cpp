#include "vulkan_utils/swap_chain.hpp"
#include "vulkan_utils/create_info.hpp"
#include <utility>

namespace swap_chain {
SwapChain::SwapChain(GLFWwindow *m_window, VkSurfaceKHR m_surface,
                     device::DeviceHandler *m_deviceHandler)
    : m_window(m_window), m_surface(m_surface),
      m_deviceHandler(m_deviceHandler) {
    createSwapChain();
    createImageViews();
    createRenderPass();
    createFrameBuffers();
};

void SwapChain::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {

        VkImageViewCreateInfo createInfo = create_info::imageViewCreateInfo(
            swapChainImages[i], swapChainImageFormat);

        VK_CHECK(vkCreateImageView(m_deviceHandler->logicalDevice, &createInfo,
                                   nullptr, &swapChainImageViews[i]));
    }
}

void SwapChain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport =
        m_deviceHandler->querySwapChainSupport(m_deviceHandler->physicalDevice);

    VkSurfaceFormatKHR surfaceFormat =
        m_chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode =
        m_chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = m_chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    QueueFamilyIndices indices =
        m_deviceHandler->getQueueFamilyIndices(m_deviceHandler->physicalDevice);

    VkSwapchainCreateInfoKHR createInfo = create_info::swapChainCreateInfo(
        m_surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
        extent, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, indices,
        swapChainSupport.capabilities.currentTransform, presentMode);

    VK_CHECK(vkCreateSwapchainKHR(m_deviceHandler->logicalDevice, &createInfo,
                                  nullptr, &swapChain));

    vkGetSwapchainImagesKHR(m_deviceHandler->logicalDevice, swapChain,
                            &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_deviceHandler->logicalDevice, swapChain,
                            &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void SwapChain::createRenderPass() {
    VkRenderPass renderPass;
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(m_deviceHandler->logicalDevice, &renderPassInfo,
                                nullptr, &renderPass));
    renderPasses.push_back(renderPass);
}

int SwapChain::createRenderPass(VkRenderPassCreateInfo renderPassInfo) {
    VkRenderPass renderPass;
    VK_CHECK(vkCreateRenderPass(m_deviceHandler->logicalDevice, &renderPassInfo,
                                nullptr, &renderPass));
    renderPasses.push_back(renderPass);
    return renderPasses.size() - 1;
}

void SwapChain::createFrameBuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 1> attachments = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPasses[0];
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(m_deviceHandler->logicalDevice,
                                     &framebufferInfo, nullptr,
                                     &swapChainFramebuffers[i]));
    }
}

VkSurfaceFormatKHR SwapChain::m_chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::m_chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &presentMode : availablePresentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
SwapChain::m_chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    int width;
    int height;
    glfwGetFramebufferSize(m_window, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

void SwapChain::cleanup() {
    for (auto *framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(m_deviceHandler->logicalDevice, framebuffer,
                             nullptr);
    }
    swapChainFramebuffers.clear();

    for (auto *renderPass : renderPasses) {
        vkDestroyRenderPass(m_deviceHandler->logicalDevice, renderPass,
                            nullptr);
    }
    renderPasses.clear();

    for (auto *imageView : swapChainImageViews) {
        vkDestroyImageView(m_deviceHandler->logicalDevice, imageView, nullptr);
    }
    swapChainImageViews.clear();

    vkDestroySwapchainKHR(m_deviceHandler->logicalDevice, swapChain, nullptr);
}
} // namespace swap_chain
