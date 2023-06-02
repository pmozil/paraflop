#pragma once
#include "common.hpp"
#include "vulkan_utils/device.hpp"

namespace swap_chain {
/**
 * \class SwapChain
 *
 * \brief Represents the swap chain used for rendering in Vulkan.
 *
 * The SwapChain class provides functionality for creating and managing the swap
 * chain in Vulkan. The swap chain is responsible for presenting rendered images
 * on the screen. This class encapsulates the swap chain object and associated
 * resources such as render passes, images, image views, and framebuffers.
 */
class SwapChain {
  public:
    /**
     * \fn SwapChain(GLFWwindow *m_window, VkSurfaceKHR m_surface,
     *               std::shared_ptr<device::DeviceHandler> m_deviceHandler)
     *
     * \brief Constructs a SwapChain object.
     *
     * \param m_window The GLFW window associated with the swap chain.
     * \param m_surface The Vulkan surface associated with the swap chain.
     * \param m_deviceHandler The device handler associated with the swap chain.
     */
    SwapChain(GLFWwindow *m_window, VkSurfaceKHR m_surface,
              std::shared_ptr<device::DeviceHandler> m_deviceHandler);

    /**
     * \fn ~SwapChain()
     *
     * \brief Destructor for the SwapChain class.
     */
    ~SwapChain() {
        cleanup();
        m_destroySyncObjects();
    }

    VkSwapchainKHR swapChain;                     /**< The swap chain */
    VkFormat swapChainImageFormat;                /**< The image format */
    VkExtent2D swapChainExtent;                   /**< The swap chain extent */
    std::vector<VkRenderPass> renderPasses;       /**< The render pass */
    std::vector<VkImage> swapChainImages;         /**< The swap chain images */
    std::vector<VkImageView> swapChainImageViews; /**< The image views */
    std::vector<VkFramebuffer> swapChainFramebuffers; /**< The image buffers */
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;

    /**
     * \fn VkRenderPass getRenderPass()
     *
     * \brief Retrieves the first render pass in the render passes vector.
     *
     * \return The first render pass in the render passes vector.
     */
    inline VkRenderPass getRenderPass() { return renderPasses[0]; };

    /**
     * \fn VkRenderPass getRenderPass(int n)
     *
     * \brief Retrieves the render pass at the specified index in the render
     * passes vector.
     *
     * \param n The index of the render pass to retrieve.
     *
     * \return The render pass at the specified index.
     */
    inline VkRenderPass getRenderPass(int n) { return renderPasses[n]; };

    /**
     * \fn VkResult getNextImage()
     *
     * \brief A wrapper for VkAcquireNextImageKHR
     *
     * \param frameIdx The frame index
     * \param imageIndex A pointer to int, containing new image index
     *
     * \return A VkResult - mey not be an image
     */
    VkResult getNextImage(uint32_t frameIdx, uint32_t *imageIndex);

    /**
     * \fn int renderPassesLength() const
     *
     * \brief Retrieves the number of render passes in the render passes vector.
     *
     * \return The number of render passes.
     */
    [[nodiscard]] inline int renderPassesLength() const {
        return renderPasses.size();
    };

    /**
     * \fn int createRenderPass(VkRenderPassCreateInfo renderPassInfo)
     *
     * \brief Creates a new render pass and adds it to the render passes vector.
     *
     * \param renderPassInfo The render pass creation information.
     *
     * \return The index of the newly created render pass in the render passes
     * vector.
     */
    int createRenderPass(VkRenderPassCreateInfo renderPassInfo);

    /**
     * \fn void cleanup()
     *
     * \brief Cleans up the resources associated with the swap chain.
     */
    void cleanup();

    /**
     * \fn void createSwapChain()
     *
     * \brief Creates the swap chain.
     */
    void createSwapChain();

    /**
     * \fn void createImageViews()
     *
     * \brief Creates the image views for the swap chain images.
     */
    void createImageViews();

    /**
     * \fn void createRenderPass()
     *
     * \brief Creates the render passes for the swap chain.
     */
    void createRenderPass();

    /**
     * \fn void createFrameBuffers()
     *
     * \brief Creates the framebuffers for the swap chain.
     */
    void createFrameBuffers();

  private:
    GLFWwindow *m_window;
    VkSurfaceKHR m_surface;
    std::shared_ptr<device::DeviceHandler> m_deviceHandler;

    /**
     * \fn static VkSurfaceFormatKHR m_chooseSwapSurfaceFormat(const
     * std::vector<VkSurfaceFormatKHR> &availableFormats)
     *
     * \brief Chooses the surface format for the swap chain.
     *
     * \param availableFormats The available surface formats.
     *
     * \return The chosen surface format.
     */
    static VkSurfaceFormatKHR m_chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);

    /**
     * \fn static VkPresentModeKHR m_chooseSwapPresentMode(const
     * std::vector<VkPresentModeKHR> &availablePresentModes)
     *
     * \brief Chooses the presentation mode for the swap chain.
     *
     * \param availablePresentModes The available presentation modes.
     *
     * \return The chosen presentation mode.
     */
    static VkPresentModeKHR m_chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);

    /**
     * \fn VkExtent2D m_chooseSwapExtent(const VkSurfaceCapabilitiesKHR
     * &capabilities)
     *
     * \brief Chooses the swap extent for the swap chain.
     *
     * \param capabilities The capabilities of the surface.
     *
     * \return The chosen swap extent.
     */
    VkExtent2D m_chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    /**
     * \fn void m_createSyncObjects()
     *
     * \brief Creates synchronization objects.
     *
     * This function creates the semaphores and fences used for synchronization
     * between the CPU and GPU.
     */
    void m_createSyncObjects();

    /**
     * \fn void m_destroySyncObjects();
     *
     * \brief Clean up sync objects
     */
    void m_destroySyncObjects();
};
} // namespace swap_chain
