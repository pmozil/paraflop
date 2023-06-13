#pragma once

#include "gltf_model/gltf_common.hpp"

namespace gltf_model {
/*
    glTF animation channel
*/
struct AnimationChannel {
    enum PathType { TRANSLATION, ROTATION, SCALE };
    PathType path;
    Node *node;
    uint32_t samplerIndex;
};

/*
    glTF animation sampler
*/
struct AnimationSampler {
    enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
    InterpolationType interpolation;
    std::vector<float> inputs;
    std::vector<glm::vec4> outputsVec4;
};

/*
    glTF animation
*/
struct Animation {
    std::string name;
    std::vector<AnimationSampler> samplers;
    std::vector<AnimationChannel> channels;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::min();
};
} // namespace gltf_model
