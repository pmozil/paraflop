/*
 * Vulkan glTF model and texture loading class based on tinyglTF
 * (https://github.com/syoyo/tinygltf)
 *
 * Copyright (C) 2018 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT)
 * (http://opensource.org/licenses/MIT)
 */

#include "gltf_model/primitive.hpp"
#include "vulkan_utils/create_info.hpp"
#include "vulkan_utils/staging_buffer.hpp"
#include "vulkan_utils/utils.hpp"

namespace gltf_model {
/*
    glTF primitive
*/
void gltf_model::Primitive::setDimensions(glm::vec3 min, glm::vec3 max) {
    dimensions.min = min;
    dimensions.max = max;
    dimensions.size = max - min;
    dimensions.center =
        (min + max) * HALF; /**< Average vectors for their centroid */
    dimensions.radius = glm::distance(min, max) * HALF;
}

} // namespace gltf_model
