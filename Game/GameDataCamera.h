#pragma once

#include <glm/glm.hpp>

#include "GameData.h"

struct GameDataCamera : public GameData
{
    GameDataCamera()
    {
        size = sizeof(Camera);
    }

    void update(GameDataContainer* container, uint32_t frameID, uint32_t dataID, GameState& game, bool alignToFourBytes = true) override;
};