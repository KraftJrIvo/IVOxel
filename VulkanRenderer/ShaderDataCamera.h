#pragma once

#include <glm/glm.hpp>

#include "ShaderData.h"

struct ShaderDataCamera : public ShaderData
{
    struct State
    {
        glm::mat4 mvp;
        glm::vec2 resolution;
        float time;
        float fov;
        glm::vec3 pos;
    };
    State state;

    ShaderDataCamera()
    {
        size = sizeof(State);
    }

    void update(VulkanDescriptorPool& pool, uint32_t frameID, uint32_t dataID, GameState& game) override;
};