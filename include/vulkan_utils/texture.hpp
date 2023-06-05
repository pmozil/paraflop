/**
 * Most iof this code has been taken from Sacha Willems' Samples.
 */
#pragma once
#include "common.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/staging_buffer.hpp"

namespace texture {
/**
 * \file
 * \brief Defines classes and methods related to texture handling in Vulkan
 * graphics API.
 */

/**
 * \class Texture
 * \brief Represents a generic texture object in Vulkan.
 *
 * This class provides the basic functionality for managing a texture in Vulkan,
 * including loading, updating, and destroying the texture resources.
 */
class Texture {
  public:
    VkImage image;             /**< Vulkan image object. */
    VkImageLayout imageLayout; /**< Current layout of the image. */
    VkDeviceMemory
        deviceMemory; /**< Device memory object associated with the image. */
    VkImageView view; /**< Image view object. */
    uint32_t width, height; /**< Dimensions of the texture. */
    uint32_t mipLevels;     /**< Number of mip levels in the texture. */
    uint32_t layerCount;    /**< Number of layers in the texture. */
    VkDescriptorImageInfo descriptor; /**< Descriptor image information. */
    VkSampler sampler;                /**< Texture sampler object. */

    /**
     * \fn void Texture::updateDescriptor()
     *
     * \brief Updates the descriptor image information for the texture.
     *
     * This method updates the descriptor image information for the texture,
     * which is used for binding the texture to shader resources.
     */
    void updateDescriptor();

    /**
     * \fn void destroy()
     *
     * \brief Destroys the texture resources.
     *
     * This method releases the Vulkan resources associated with the texture,
     * freeing up the memory.
     */
    void destroy();

    /**
     * \fn ktxResult loadKTXFile(const std::string &filename, ktxTexture
     * **target)
     *
     * \brief Loads a KTX file as a texture.
     *
     * \param filename The path to the KTX file.
     * \param target Pointer to store the loaded KTX texture data.
     * \return The result of the KTX file loading operation.
     */
    static ktxResult loadKTXFile(const std::string &filename,
                                 ktxTexture **target);

  protected:
    std::shared_ptr<device::DeviceHandler>
        m_deviceHandler; /**< Device handler for
                            Vulkan operations. */
    std::shared_ptr<command_buffer::CommandBufferHandler>
        m_commandBufferHandler;
};

/**
 * \class Texture2D
 * \brief Represents a 2D texture in Vulkan.
 *
 * This class extends the Texture class and provides additional functionality
 * specific to 2D textures.
 */
class Texture2D : public Texture {
  public:
    /**
     * \fn Texture2D(
     *      const std::string &filename, VkFormat format,
     *      std::shared_ptr<device::DeviceHandler> device,
     *      VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
     *      VkImageLayout imageLayout =
     * VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, bool forceLinear = false)
     *
     * \brief Loads a texture from a file.
     *
     * \param filename The path to the texture file.
     * \param format The format of the texture.
     * \param device The device handler for Vulkan operations.
     * \param imageUsageFlags The usage flags for the image.
     * \param imageLayout The initial layout of the image.
     * \param forceLinear Flag indicating whether to force linear tiling for the
     * image.
     */
    Texture2D(
        const std::string &filename, VkFormat format,
        std::shared_ptr<device::DeviceHandler> deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler>
            commandBufferHandler,
        VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        bool forceLinear = false);

    /**
     * \fn Texture2D(
     *      void *buffer, VkDeviceSize bufferSize, VkFormat format,
     *      uint32_t texWidth, uint32_t texHeight,
     *      std::shared_ptr<device::DeviceHandler> device,
     *      VkFilter filter = VK_FILTER_LINEAR,
     *      VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
     *      VkImageLayout imageLayout =
     * VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
     *
     * \brief Creates a 2D texture from a buffer.
     *
     * \param buffer Pointer to the buffer containing the texture data.
     * \param bufferSize Size of the buffer.
     * \param format The format of the texture.
     * \param texWidth Width of the texture.
     * \param texHeight Height of the texture.
     * \param device The device handler for Vulkan operations.
     * \param filter The texture filtering mode.
     * \param imageUsageFlags The usage flags for the image.
     * \param imageLayout The initial layout of the image.
     */
    Texture2D(
        void *buffer, VkDeviceSize bufferSize, VkFormat format,
        uint32_t texWidth, uint32_t texHeight,
        std::shared_ptr<device::DeviceHandler> device,
        std::shared_ptr<command_buffer::CommandBufferHandler>
            m_commandBufferHandler,
        VkFilter filter = VK_FILTER_LINEAR,
        VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    /** \fn ~Texture2D()
     *
     * \brief Destructor for Texture2D
     */
    ~Texture2D() { destroy(); }
};

/**
 * \class Texture2DArray
 * \brief Represents a 2D array texture in Vulkan.
 *
 * This class extends the Texture class and provides additional functionality
 * specific to 2D array textures.
 */
class Texture2DArray : public Texture {
  public:
    /**
     * \fn void Texture2DArray::loadFromFile(
     *      const std::string &filename, VkFormat format,
     *      std::shared_ptr<device::DeviceHandler> device,
     *      VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
     *      VkImageLayout imageLayout =
     * VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
     *
     * \brief Loads a texture array from a file.
     *
     * \param filename The path to the texture file.
     * \param format The format of the texture.
     * \param device The device handler for Vulkan operations.
     * \param imageUsageFlags The usage flags for the image.
     * \param imageLayout The initial layout of the image.
     */
    Texture2DArray(
        const std::string &filename, VkFormat format,
        std::shared_ptr<device::DeviceHandler> m_deviceHandler,
        std::shared_ptr<command_buffer::CommandBufferHandler>
            m_commandBufferHandler,
        VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
};

} // namespace texture
