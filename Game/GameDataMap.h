#pragma once

#include <glm/glm.hpp>

#include "GameData.h"

struct GameDataMap : public GameData
{
    GameDataMap(uint32_t size);

    void update(GameState& game, bool alignToFourBytes = true) override;
    void upload(GameState& game, uint32_t dataID, GameDataContainer* container = nullptr, uint32_t frameID = 0) override;
};