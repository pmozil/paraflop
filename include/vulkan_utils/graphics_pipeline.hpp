#pragma once
#include "common.hpp"
#include "geometry/vertex.hpp"
#include "vulkan_utils/device.hpp"
#include "vulkan_utils/swap_chain.hpp"

namespace graphics_pipeline {
/**
 * \class AbstractGraphicsPipeline
 * \brief Base class for graphics pipeline creation.
 *
 * The AbstractGraphicsPipeline class provides an interface for creating and
 * managing Vulkan graphics pipelines.
 */
class AbstractGraphicsPipeline {
  public:
    /**
     * \brief Creates the graphics pipeline.
     */
    virtual void createGraphicsPipeline() = 0;

    VkPipelineLayout pipelineLayout; /**< The pipeline layout. */
    VkPipeline graphicsPipeline;     /**< The graphics pipeline. */

    /**
     * \brief Performs cleanup operations for the graphics pipeline.
     */
    void cleanup();

    /**
     * \brief Reads a file and returns its contents as a vector of characters.
     *
     * \param filename The name of the file to read.
     * \return A vector of characters containing the file contents.
     */
    static std::vector<char> readFile(const std::string &filename);

    /**
     * \brief Creates a shader module from the given code.
     *
     * \param code The code representing the shader.
     * \return The created shader module.
     */
    VkShaderModule createShaderModule(const std::vector<char> &code);

  protected:
    /**
     * \brief Constructs an AbstractGraphicsPipeline object.
     *
     * \param m_swapChain The swap chain associated with the pipeline.
     * \param m_deviceHandler The device handler used for pipeline operations.
     * \param layout The descriptor set layout used by the pipeline.
     */
    AbstractGraphicsPipeline(
        std::shared_ptr<swap_chain::SwapChain> m_swapChain,
        std::shared_ptr<device::DeviceHandler> m_deviceHandler,
        VkDescriptorSetLayout *layout)
        : m_descriptorSetLayout(layout), m_swapChain(std::move(m_swapChain)),
          m_deviceHandler(std::move(m_deviceHandler)){};

    VkDescriptorSetLayout *m_descriptorSetLayout; /**< The descriptor set layout
                                                     used by the pipeline. */
    std::shared_ptr<swap_chain::SwapChain>
        m_swapChain; /**< The swap chain associated with the pipeline. */
    std::shared_ptr<device::DeviceHandler>
        m_deviceHandler; /**< The device handler used for pipeline operations.
                          */

    ~AbstractGraphicsPipeline() { cleanup(); }
};

/**
 * \class CustomGraphicsPipeline
 * \brief Custom implementation of a graphics pipeline.
 *
 * The CustomGraphicsPipeline class extends the AbstractGraphicsPipeline class
 * to provide a custom implementation of a graphics pipeline.
 */
class CustomGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    /**
     * \brief Constructs a CustomGraphicsPipeline object.
     *
     * \param m_swapChain The swap chain associated with the pipeline.
     * \param m_deviceHandler The device handler used for pipeline operations.
     */
    CustomGraphicsPipeline(
        std::shared_ptr<swap_chain::SwapChain> m_swapChain,
        std::shared_ptr<device::DeviceHandler> m_deviceHandler);

    /**
     * \fn void createGraphicsPipelineFromInfo(
     *     VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo,
     *     VkGraphicsPipelineCreateInfo pipelineCreateInfo)
     *
     * \brief Creates the graphics pipeline from the provided pipeline layout
     * and pipeline create info.
     *
     * \param pipelineLayoutCreateInfo The create info for the pipeline layout.
     * \param pipelineCreateInfo The create info for the graphics pipeline.
     */
    void createGraphicsPipelineFromInfo(
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo,
        VkGraphicsPipelineCreateInfo pipelineCreateInfo);

    /**
     * \fn ~CustomGraphicsPipeline()
     *
     * \brief Destructor for the CustomGraphicsPipeline class.
     */
    ~CustomGraphicsPipeline() { cleanup(); }
};

/**
 * \class RasterGraphicsPipeline
 * \brief Raster graphics pipeline implementation.
 *
 * The RasterGraphicsPipeline class extends the AbstractGraphicsPipeline class
 * and provides an implementation for a raster graphics pipeline.
 */
class RasterGraphicsPipeline : public AbstractGraphicsPipeline {
  public:
    /**
     * \brief Constructs a RasterGraphicsPipeline object.
     *
     * \param m_swapChain The swap chain associated with the pipeline.
     * \param m_deviceHandler The device handler used for pipeline operations.
     */
    RasterGraphicsPipeline(
        std::shared_ptr<swap_chain::SwapChain> m_swapChain,
        std::shared_ptr<device::DeviceHandler> m_deviceHandler);

    /**
     * \fn void createGraphicsPipeline() override
     *
     * \brief Creates the graphics pipeline.
     *
     * This function overrides the pure virtual function in the base class to
     * create the graphics pipeline.
     */
    void createGraphicsPipeline() override;

    /**
     * \fn ~RasterGraphicsPipeline()
     *
     * \brief Destructor for the RasterGraphicsPipeline class.
     */
    ~RasterGraphicsPipeline() { cleanup(); }
};

} // namespace graphics_pipeline
