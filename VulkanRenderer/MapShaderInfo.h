#pragma once

#include <glm/glm.hpp>

struct MapShaderInfo {
    glm::vec4 chunkOffsets[8];
    glm::uvec4 chunkData[4096 / 16];
};