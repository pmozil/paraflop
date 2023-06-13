#pragma once
#include "common.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

namespace buffer {
/**
 * \class Buffer
 *
 * \brief Represents a Vulkan buffer with associated memory.
 *
 * This class encapsulates a Vulkan buffer along with its memory allocation and
 * provides various operations for managing and manipulating the buffer data.
 */
class Buffer {
  public:
    /**
     * \brief Constructs a Buffer object.
     *
     * \param m_devicehandler The device handler used to create the buffer.
     * \param m_commandBuffer The command buffer handler associated with the
     * buffer. \param usageFlags The usage flags specifying how the buffer will
     * be used. \param memoryPropertyFlags The memory property flags for the
     * buffer memory. \param sharingMode The sharing mode of the buffer. \param
     * size The size of the buffer in bytes.
     */
    Buffer(
        std::shared_ptr<device::DeviceHandler> m_devicehandler,
        std::shared_ptr<command_buffer::CommandBufferHandler> m_commandBuffer,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags, VkSharingMode sharingMode,
        VkDeviceSize size);

    Buffer() = default;

    /**
     * \brief Destroys the Buffer object.
     *
     * This destructor releases the resources associated with the buffer.
     */
    ~Buffer() { destroy(); }

    /**
     * \fn void map()
     *
     * \brief Maps the buffer memory into host-accessible memory.
     *
     * This function maps the buffer memory into host-accessible memory,
     * allowing the host to read or modify the buffer's data.
     */
    void map();

    /**
     * \fn unmap()
     *
     * \brief Unmaps the buffer memory.
     *
     * This function unmaps the previously mapped buffer memory.
     */
    void unmap();

    /**
     * \fn bind(VkDeviceSize offset)
     *
     * \brief Binds the buffer to the specified offset.
     *
     * This function binds the buffer to the specified offset, making it the
     * active buffer in the command buffer.
     *
     * \param offset The offset within the buffer to bind.
     */
    void bind(VkDeviceSize offset = 0);

    /**
     * \fn void setupDescriptor()
     *
     * \brief Sets up the descriptor for the buffer.
     *
     * This function sets up the descriptor for the buffer, which is used to
     * describe the buffer in shader bindings.
     */
    void setupDescriptor();

    /**
     * \fn void copy(void *data, VkDeviceSize size)
     *
     * \brief Copies data to the buffer with the stage buffer.
     *
     * This function copies data to the buffer from the provided source data
     * pointer.
     *
     * \param data Pointer to the source data to be copied.
     * \param size The size of the data to be copied in bytes.
     */
    void copy(void *data, VkDeviceSize size);

    /**
     * \fn void fastCopy(void *data, VkDeviceSize size)
     *
     * \brief Copies data to the buffer without the stage buffer.
     *
     * This function performs a fast memory copy of data to the buffer.
     * The function assumes that the buffer is already mapped.
     *
     * \param data Pointer to the source data to be copied.
     * \param size The size of the data to be copied in bytes.
     */
    void fastCopy(void *data, VkDeviceSize size);

    /**
     * \fn void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset =
     * 0)
     *
     * \brief Flushes the buffer memory ranges to make them visible to the
     * device.
     *
     * This function flushes the specified range of buffer memory to make it
     * visible to the device.
     *
     * \param size The size of the memory range to flush in bytes.
     * \param offset The offset within the buffer to start flushing from.
     */
    void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    /**
     * \fn void invalidate(VkDeviceSize size = VK_WHOLE_SIZE,
     * VkDeviceSize offset = 0)
     *
     * \brief Invalidates the buffer memory ranges to make them coherent with
     * the device.
     *
     * This function invalidates the specified range of buffer memory to make it
     * coherent with the device.
     *
     * \param size The size of the memory range to invalidate in bytes.
     * \param offset The offset within the buffer to start invalidating from.
     */
    void invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    /**
     * \fn void copyFrom(VkBuffer srcBuffer)
     *
     * \brief Copies data from another buffer.
     *
     * This function copies data from the specified source buffer to this
     * buffer.
     *
     * \param srcBuffer The source buffer to copy from.
     */
    void copyFrom(VkBuffer srcBuffer);

    /**
     * \fn void copyTo(VkBuffer dstBuffer)
     *
     * \brief Copies data to another buffer.
     *
     * This function copies data from this buffer to the specified destination
     * buffer.
     *
     * \param dstBuffer The destination buffer to copy to.
     */
    void copyTo(VkBuffer dstBuffer);

    /**
     * \fn void destroy()
     *
     * \brief Destroys the buffer and frees associated resources.
     *
     * This function destroys the buffer object and frees the associated Vulkan
     * resources.
     */
    void destroy();

    /**
     * \fn VkBuffer()
     *
     * \brief Operator for Buffer to be used as VkBuffer
     *
     * \return The buffer that the class wraps
     */
    operator VkBuffer() const { return buffer; }

    VkBuffer buffer = VK_NULL_HANDLE; /**< The Vulkan buffer handle. */
    VkDeviceMemory memory =
        VK_NULL_HANDLE;                /**< The Vulkan device memory handle. */
    VkDescriptorBufferInfo descriptor; /**< Descriptor for the buffer. */
    VkDeviceSize size = 0;             /**< Size of the buffer in bytes. */
    VkDeviceSize alignment = 0; /**< Alignment requirement for the buffer. */
    void *mapped = nullptr;     /**< Pointer to the mapped buffer memory. */
    VkBufferUsageFlags
        usageFlags; /**< Usage flags specifying how the buffer is used. */
    VkMemoryPropertyFlags memoryPropertyFlags; /**< Memory property flags for
                                                  the buffer memory. */

  private:
    /**
     * \fn void m_makeBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory, VkSharingMode sharingMode)
     *
     * \brief Creates a buffer with the specified properties.
     *
     * This function creates a buffer with the specified size, usage flags, and
     * memory properties.
     *
     * \param size The size of the buffer in bytes.
     * \param usage The usage flags specifying how the buffer will be used.
     * \param properties The memory property flags for the buffer memory.
     * \param buffer [out] The created Vulkan buffer handle.
     * \param bufferMemory [out] The allocated Vulkan device memory handle.
     * \param sharingMode The sharing mode of the buffer.
     */
    void m_makeBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory, VkSharingMode sharingMode);

    std::shared_ptr<command_buffer::CommandBufferHandler>
        m_commandBuffer; /**< Command buffer handler associated with the buffer.
                          */
    std::shared_ptr<device::DeviceHandler>
        m_deviceHandler; /**< Device handler used to create the buffer. */
};
} // namespace buffer
