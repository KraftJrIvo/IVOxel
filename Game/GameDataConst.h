#pragma once

#include <glm/glm.hpp>

#include "GameData.h"

struct GameDataConst : public GameData
{
    GameDataConst(uint8_t chunkLoadRadius, uint8_t maxLights, float epsilon);

	uint8_t _chunkLoadRadius;
	uint8_t _maxLights;
	float _epsilon;

    void update(GameState& game, bool alignToFourBytes = true) override;
    void upload(GameState& game, uint32_t dataID, GameDataContainer* container = nullptr, uint32_t frameID = 0) override;
};