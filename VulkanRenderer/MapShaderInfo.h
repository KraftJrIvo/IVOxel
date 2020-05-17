#pragma once

#include <glm/glm.hpp>

struct MapShaderInfo {
    glm::vec4 chunkOffsets[8];
    char chunkData[sizeof(int32_t) * 1024];
};