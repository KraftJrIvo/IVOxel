#pragma once

#include <glm/glm.hpp>

struct ViewShaderInfo {
    glm::mat4 mvp;
    glm::vec2 resolution;
    float time;
    float fov;
    glm::vec3 pos;
};