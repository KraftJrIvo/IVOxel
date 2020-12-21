#pragma once

#include <glm/glm.hpp>

#include "ShaderData.h"

struct ShaderDataMap : public ShaderData
{
    struct State
    {
        glm::vec4 chunkOffsets[8];
        glm::uvec4 chunkData[4096 / 16];
    };
    State state;

    ShaderDataMap()
    {
        size = sizeof(State);
    }

    void update(VulkanDescriptorPool& pool, uint32_t frameID, uint32_t dataID, GameState& game) override;
};