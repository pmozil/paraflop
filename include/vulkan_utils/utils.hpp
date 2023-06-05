/* All the code in this file is taken from Sacha Willems' code.
 * Check his repo out, it's very cool
 */
#pragma once
#include "common.hpp"

namespace utils {
/**
 * \fn bool fileExists(const std::string &filename)
 *
 * \brief Check if file exists
 *
 * \param filename The file to be checked
 */
bool fileExists(const std::string &filename);

/**
 * \fn void exitFatal(const std::string& message, int32_t exitCode)
 *
 * \brief Closes the program
 *
 * \param message The error message
 * \param exitCode The exit code
 */
void exitFatal(const std::string &message, int32_t exitCode);

// Put an image memory barrier for setting an image layout on the sub resource
// into the given command buffer
void setImageLayout(
    VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

// Uses a fixed sub resource layout with first mip level and layer
void setImageLayout(
    VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask,
    VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

} // namespace utils
