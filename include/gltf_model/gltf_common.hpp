
/*
 * Vulkan glTF model and texture loading class based on tinyglTF
 * (https://github.com/syoyo/tinygltf)
 *
 * Copyright (C) 2018 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT)
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include "common.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

// Changing this value here also requires changing it in the vertex shader
#define MAX_JOINTS 128u

namespace gltf_model {
enum DescriptorBindingFlags {
    ImageBaseColor = 0x00000001,
    ImageNormalMap = 0x00000002
};

extern VkDescriptorSetLayout descriptorSetLayoutImage;
extern VkDescriptorSetLayout descriptorSetLayoutUbo;
extern VkMemoryPropertyFlags memoryPropertyFlags;
extern uint32_t descriptorBindingFlags;

VkDescriptorSetLayout descriptorSetLayoutImage = VK_NULL_HANDLE;
VkDescriptorSetLayout descriptorSetLayoutUbo = VK_NULL_HANDLE;
VkMemoryPropertyFlags memoryPropertyFlags = 0;
uint32_t descriptorBindingFlags =
    gltf_model::DescriptorBindingFlags::ImageBaseColor;

struct Node;

enum FileLoadingFlags {
    None = 0x00000000,
    PreTransformVertices = 0x00000001,
    PreMultiplyVertexColors = 0x00000002,
    FlipY = 0x00000004,
    DontLoadImages = 0x00000008
};

enum RenderFlags {
    BindImages = 0x00000001,
    RenderOpaqueNodes = 0x00000002,
    RenderAlphaMaskedNodes = 0x00000004,
    RenderAlphaBlendedNodes = 0x00000008
};

} // namespace gltf_model
