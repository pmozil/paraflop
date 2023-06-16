#pragma once

#include "gltf_model/gltf_common.hpp"

namespace gltf_model {

static const float MAX_ANISOTROPY = 8.0F;
/*
    glTF texture loading class
*/
struct Texture {
    std::shared_ptr<device::DeviceHandler> deviceHandler;
    std::shared_ptr<command_buffer::CommandBufferHandler> commandBuffer;
    std::string name;
    VkImage image;
    VkImageLayout imageLayout;
    VkDeviceMemory deviceMemory;
    VkImageView view;
    uint32_t width, height;
    uint32_t mipLevels;
    uint32_t layerCount;
    VkDescriptorImageInfo descriptor;
    VkSampler sampler;
    void updateDescriptor();
    void destroy();
    void
    fromglTfImage(tinygltf::Image &gltfimage, std::string &path,
                  std::shared_ptr<device::DeviceHandler> device,
                  std::shared_ptr<command_buffer::CommandBufferHandler> cmdBuf,
                  VkQueue copyQueue);

    void makeglTFImage(tinygltf::Image &gltfimage, VkFormat &format,
                       VkQueue copyQueue);

    void makeBlits(VkImageSubresourceRange &subresourceRange,
                   VkQueue copyQueue);

    void makeKTXImage(const std::string &filename, VkFormat &format,
                      VkQueue copyQueue);
};
} // namespace gltf_model
