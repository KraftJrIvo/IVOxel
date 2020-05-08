#pragma once

#include <glm/glm.hpp>

#include "VulkanTypes.h"

struct ShaderInfoPackage {
    glm::mat4 mvp;
    glm::vec2 resolution;
    float time;
};