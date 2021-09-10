#pragma once

#include <glm/glm.hpp>

#include "GameData.h"
#include "Camera.h"

struct GameDataCamera : public GameData
{
    GameDataCamera();

    void update(GameState& game, bool alignToFourBytes = true) override;
    void upload(GameState& game, uint32_t dataID, GameDataContainer* container = nullptr, uint32_t frameID = 0) override;
};