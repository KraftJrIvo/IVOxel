#pragma once

#include <glm/glm.hpp>

struct LightingShaderInfo {
    glm::vec4 coords[16];
    glm::vec4 colors[16];
    int32_t nLights;
};