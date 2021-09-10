#pragma once

#include <glm/glm.hpp>

#include "GameData.h"

struct GameDataLight : public GameData
{
    GameDataLight(uint8_t maxLights);

    void update(GameState& game, bool alignToFourBytes = true) override;
    void upload(GameState& game, uint32_t dataID, GameDataContainer* container = nullptr, uint32_t frameID = 0) override;
};