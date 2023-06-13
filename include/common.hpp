#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ktx.h>
#include <ktxvulkan.h>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <thread>
#include <type_traits>
// Window dimensions
const int WIDTH = 800;
const int HEIGHT = 600;

// Maximum number of frames in flight
const int MAX_FRAMES_IN_FLIGHT = 3;

// Validation layers
const std::vector<const char *> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation",
};

// // Device extensions required for swap chain
// const std::vector<const char *> DEVICE_EXTENSIONS = {
//     VK_KHR_SWAPCHAIN_EXTENSION_NAME,
//     VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
//     VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
//     VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
//     VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
//     VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
//     VK_KHR_SPIRV_1_4_EXTENSION_NAME,
//     VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME};

// Vulkan API version and global version
#define PARAFLOP_VK_API_VERSION VK_API_VERSION_1_1
#define GLOBAL_VERSION VK_MAKE_VERSION(1, 0, 0)

#define UNUSED(x) (void)(x)

// Enable or disable validation layers based on build mode
#define NDEBUG

#ifdef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

// Data structure to store queue family indices
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> transferFamily;

    // Check if all required queue families are present
    [[nodiscard]] inline bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    // Check if a dedicated transfer queue family is available
    [[nodiscard]] inline bool hasDedicatedTransfer() const {
        return transferFamily.has_value();
    }
};

// Structure to store swap chain support details
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// Macro for checking Vulkan function calls for errors
#define VK_CHECK(f)                                                            \
    {                                                                          \
        VkResult res = (f);                                                    \
        if (res != VK_SUCCESS) {                                               \
            std::cout << "Fatal : VkResult is \"" << res << "\" in "           \
                      << __FILE__ << " at line " << __LINE__ << "\n";          \
            assert(res == VK_SUCCESS);                                         \
        }                                                                      \
    }

// Alignment requirement for GLM types in bytes
static const size_t GLM_16_BYTE_ALIGN = 16;

// Up vector in the world coordinate system
static const glm::vec3 WORLD_UP = glm::vec3({0.0F, 1.0F, 0.0F});

// Custom define for better code readability
const uint32_t VK_FLAGS_NONE = 0;
// Default fence timeout in nanoseconds
const uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;
