#pragma once
#include "custom_graphics_pipeline.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/create_info.hpp"

namespace command_buffer {
class CustomImageCommandBuffer : public ImageCommandBufferHandler {
  private:
    std::shared_ptr<graphics_pipeline::CustomRasterPipeline> m_graphicsPipeline;

  public:
    CustomImageCommandBuffer(
        std::shared_ptr<device::DeviceHandler> m_deviceHandler,
        std::shared_ptr<swap_chain::SwapChain> m_swapChain,
        std::shared_ptr<graphics_pipeline::CustomRasterPipeline>
            m_graphicsPipeline)
        : ImageCommandBufferHandler(std::move(m_deviceHandler),
                                    std::move(m_swapChain),
                                    std::move(m_graphicsPipeline)) {
        recordCommandBuffers();
    }

    void recordCommandBuffers() {
        VkCommandBufferBeginInfo cmdBufferBeginInfo{};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        uint32_t width = m_swapChain->swapChainExtent.width;
        uint32_t height = m_swapChain->swapChainExtent.height;

        VkClearValue clearValues[3];
        clearValues[0].color = {{0.0F, 0.0F, 0.0F, 1.0F}};
        clearValues[1].depthStencil = {1.0F, 0};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = m_swapChain->getRenderPass();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = width;
        renderPassBeginInfo.renderArea.extent.height = height;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;

        for (uint32_t i = 0; i < commandBuffers.size(); ++i) {
            renderPassBeginInfo.framebuffer =
                m_swapChain->swapChainFramebuffers[i];

            VkCommandBuffer currentCB = commandBuffers[i];

            VK_CHECK(vkBeginCommandBuffer(currentCB, &cmdBufferBeginInfo));
            vkCmdBeginRenderPass(currentCB, &renderPassBeginInfo,
                                 VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{};
            viewport.width = (float)width;
            viewport.height = (float)height;
            viewport.minDepth = 0.0F;
            viewport.maxDepth = 1.0F;
            vkCmdSetViewport(currentCB, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.extent = {width, height};
            vkCmdSetScissor(currentCB, 0, 1, &scissor);

            VkDeviceSize offsets[1] = {0};

            gltf_model::Model &model = *m_graphicsPipeline->model;

            vkCmdBindVertexBuffers(currentCB, 0, 1, &model.vertices.buffer,
                                   offsets);
            if (model.indices.buffer != VK_NULL_HANDLE) {
                vkCmdBindIndexBuffer(currentCB, model.indices.buffer, 0,
                                     VK_INDEX_TYPE_UINT32);
            }

            // Opaque primitives first
            for (auto *node : model.nodes) {
                model.drawNode(node, commandBuffers[i],
                               gltf_model::Material::ALPHAMODE_OPAQUE,
                               m_graphicsPipeline->pipelineLayout);
            }
            // Alpha masked primitives
            for (auto *node : model.nodes) {
                model.drawNode(node, commandBuffers[i],
                               gltf_model::Material::ALPHAMODE_MASK,
                               m_graphicsPipeline->pipelineLayout);
            }
            // Transparent primitives
            // TODO: Correct depth sorting
            for (auto *node : model.nodes) {
                model.drawNode(node, commandBuffers[i],
                               gltf_model::Material::ALPHAMODE_BLEND,
                               m_graphicsPipeline->pipelineLayout);
            }

            vkCmdEndRenderPass(currentCB);
            VK_CHECK(vkEndCommandBuffer(currentCB));
        }
    }
};
} // namespace command_buffer
