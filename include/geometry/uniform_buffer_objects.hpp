#pragma once
#include "common.hpp"

namespace geometry {
/**
 * \brief Contains transformation matrices for 3D graphics rendering.
 *
 * This struct holds three transformation matrices: model, view, and proj.
 * These matrices are used in 3D graphics rendering pipelines to transform
 * object positions from model space to screen space.
 */
struct TransformMatrices {
    alignas(GLM_16_BYTE_ALIGN) glm::mat4 model; /**< Model matrix */
    alignas(GLM_16_BYTE_ALIGN) glm::mat4 view;  /**< View matrix */
    alignas(GLM_16_BYTE_ALIGN) glm::mat4 proj;  /**< Projection matrix */
};
} // namespace geometry
