/*
 * Vulkan glTF model and texture loading class based on tinyglTF
 * (https://github.com/syoyo/tinygltf)
 *
 * Copyright (C) 2018 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT)
 * (http://opensource.org/licenses/MIT)
 */

#include "gltf_model/material.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/staging_buffer.hpp"
#include "vulkan_utils/utils.hpp"

namespace gltf_model {
/*
    glTF material
*/
void gltf_model::Material::createDescriptorSet(
    VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
    uint32_t descriptorBindingFlags) {

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};

    descriptorSetAllocInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
    descriptorSetAllocInfo.descriptorSetCount = 1;

    VK_CHECK(vkAllocateDescriptorSets(*deviceHandler, &descriptorSetAllocInfo,
                                      &descriptorSet));

    std::vector<VkDescriptorImageInfo> imageDescriptors{};
    std::vector<VkWriteDescriptorSet> writeDescriptorSets{};

    if (static_cast<bool>(descriptorBindingFlags &
                          DescriptorBindingFlags::ImageBaseColor)) {
        imageDescriptors.push_back(baseColorTexture->descriptor);
        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.dstBinding =
            static_cast<uint32_t>(writeDescriptorSets.size());
        writeDescriptorSet.pImageInfo = &baseColorTexture->descriptor;
        writeDescriptorSets.push_back(writeDescriptorSet);
    }

    if (normalTexture != nullptr &&
        static_cast<bool>(descriptorBindingFlags &
                          DescriptorBindingFlags::ImageNormalMap)) {
        imageDescriptors.push_back(normalTexture->descriptor);
        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.dstBinding =
            static_cast<uint32_t>(writeDescriptorSets.size());
        writeDescriptorSet.pImageInfo = &normalTexture->descriptor;
        writeDescriptorSets.push_back(writeDescriptorSet);
    }

    vkUpdateDescriptorSets(*deviceHandler,
                           static_cast<uint32_t>(writeDescriptorSets.size()),
                           writeDescriptorSets.data(), 0, nullptr);
}

} // namespace gltf_model
