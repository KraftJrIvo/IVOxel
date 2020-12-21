#pragma once

#include <glm/glm.hpp>

#include "ShaderData.h"

struct ShaderDataLight : public ShaderData
{
    struct State
    {
        glm::vec4 coords[16];
        glm::vec4 colors[16];
        int32_t nLights;
    };
    State state;

    ShaderDataLight()
    {
        size = sizeof(State);
    }

    void update(VulkanDescriptorPool& pool, uint32_t frameID, uint32_t dataID, GameState& game) override;
};