#pragma once

#include <stdint.h>

#include "GameDataContainer.h"

class GameState;

struct GameData
{
	virtual void update(GameDataContainer* container, uint32_t frameID, uint32_t dataID, GameState& game, bool alignToFourBytes = true) = 0;

	uint32_t size;
};
