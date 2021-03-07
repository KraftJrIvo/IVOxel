#pragma once

#include <glm/glm.hpp>

#include "GameData.h"

struct GameDataLight : public GameData
{
    glm::vec4 data[2 * 100];
    uint8_t r;

    GameDataLight(float radius) : r(radius)
    {
        size = sizeof(data);
    }

    void update(GameDataContainer* container, uint32_t frameID, uint32_t dataID, GameState& game, bool alignToFourBytes = true override;
};