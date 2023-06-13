#pragma once
#pragma GCC system_header

#include "common.hpp"
#include "vulkan_utils/command_buffer.hpp"
#include "vulkan_utils/device.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#pragma warning(disable)
#include "tiny_gltf.h"
#pragma warning(restore)
#pragma clang diagnostic pop

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
