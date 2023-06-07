#pragma once

#include "common.hpp"
#include "geometry/uniform_buffer_objects.hpp"
#include "vulkan_utils/buffer.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/device.hpp"

namespace descriptor_set {
/**
 * \class DescriptorSetLayout
 * \brief Wrapper class for Vulkan descriptor set layout.
 *
 * The DescriptorSetLayout class provides an interface for managing Vulkan
 * descriptor set layouts.
 */
class DescriptorSetLayout {
  public:
    /**
     * \brief Constructs a DescriptorSetLayout object.
     *
     * \param m_deviceHandler The device handler used to create the descriptor
     * set layout.
     * \param bindings An array of VkDescriptorSetLayoutBinding
     * structures describing the layout of the descriptor set.
     * \param bind_count The number of elements in the bindings array.
     */
    DescriptorSetLayout(std::shared_ptr<device::DeviceHandler> m_deviceHandler,
                        VkDescriptorSetLayoutBinding *bindings,
                        uint32_t bind_count);

    /**
     * \brief Destructor for the DescriptorSetLayout object.
     *
     * Cleans up the descriptor set layout resources.
     */
    ~DescriptorSetLayout() { cleanup(); }

    VkDescriptorSetLayout layout; /**< The Vulkan descriptor set layout.*/

    /**
     * \brief Cleans up the descriptor set layout resources.
     *
     * This function destroys the Vulkan descriptor set layout.
     */
    void cleanup();

    /**
     * \fn VkDescriptorSetLayout()
     *
     * \brief Operator for DescriptorSetLayout to be used as
     * VkDescriptorSetLayout
     *
     * \returns The VkDescriptorSetLayout that the class wraps
     */
    operator VkDescriptorSetLayout() const { return layout; }

  private:
    std::shared_ptr<device::DeviceHandler>
        m_deviceHandler; /**< The device handler used to create the descriptor*/
                         /**< set layout.*/

    /**
     * \brief Creates the Vulkan descriptor set layout.
     *
     * \param bindings An array of VkDescriptorSetLayoutBinding structures
     * describing the layout of the descriptor set. \param bind_count The number
     * of elements in the bindings array.
     */
    void m_createLayout(VkDescriptorSetLayoutBinding *bindings,
                        uint32_t bind_count);
};

/**
 * \class DescriptorSetHandler
 * \brief Wrapper class for Vulkan descriptor set handler.
 *
 * The DescriptorSetHandler class provides an interface for managing Vulkan
 * descriptor sets.
 */
class DescriptorSetHandler {
  public:
    /**
     * \brief Constructs a DescriptorSetHandler object.
     *
     * \param m_deviceHandler The device handler used to create the descriptor
     * set. \param m_layout The descriptor set layout associated with the
     * descriptor set. \param buffer The buffer associated with the descriptor
     * set.
     */
    DescriptorSetHandler(std::shared_ptr<device::DeviceHandler> m_deviceHandler,
                         std::shared_ptr<DescriptorSetLayout> m_layout,
                         std::vector<VkDescriptorPoolSize> sizes,
                         std::vector<VkWriteDescriptorSet> writes);

    /**
     * \brief Destructor for the DescriptorSetHandler object.
     *
     * Cleans up the descriptor set handler resources.
     */
    ~DescriptorSetHandler() { cleanup(); }

    VkDescriptorPool descriptorPool; /**< The Vulkan descriptor pool.*/
    VkDescriptorSet descriptorSet;   /**< The Vulkan descriptor set.*/
    std::vector<VkDescriptorPoolSize> sizes;
    std::vector<VkWriteDescriptorSet> writes;

    /**
     * \brief Cleans up the descriptor set handler resources.
     *
     * This function destroys the Vulkan descriptor pool.
     */
    void cleanup();

  private:
    /**
     * \brief Creates the Vulkan descriptor sets.
     *
     * This function creates the Vulkan descriptor sets using the associated
     * descriptor set layout.
     */
    void m_createDescriptorSets();

    /**
     * \brief Creates the Vulkan descriptor pool.
     *
     * This function creates the Vulkan descriptor pool for allocating
     * descriptor sets.
     */
    void m_createDescriptorPool();

    std::shared_ptr<device::DeviceHandler>
        m_deviceHandler; /**< The device handler used to create the descriptor*/
                         /**< set handler.*/
    std::shared_ptr<DescriptorSetLayout>
        m_layout; /**< The descriptor set layout associated with the
                     descriptor*/
                  /**< set.*/
    std::shared_ptr<buffer::Buffer>
        m_buffer; /**< The buffer associated with the descriptor set.*/
};

} // namespace descriptor_set
