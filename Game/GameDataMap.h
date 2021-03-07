#pragma once

#include <glm/glm.hpp>

#include "GameData.h"

struct GameDataMap : public GameData
{
    glm::vec4 data[1000];
    uint8_t r;

    GameDataMap()
    {
        size = sizeof(data);
    }

    void update(GameDataContainer* container, uint32_t frameID, uint32_t dataID, GameState& game, bool alignToFourBytes = true) override;
};