#pragma once

#include <glm/glm.hpp>

#include "VulkanTypes.h"

struct ShaderInfoPackage {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};