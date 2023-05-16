#pragma once
#include "common.hpp"

struct UniformBufferObject {
  alignas(GLM_16_BYTE_ALIGN) glm::mat4 model;
  alignas(GLM_16_BYTE_ALIGN) glm::mat4 view;
  alignas(GLM_16_BYTE_ALIGN) glm::mat4 proj;
};
