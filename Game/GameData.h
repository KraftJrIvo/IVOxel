#pragma once

#include <stdint.h>
#include <vector>

#include "GameDataContainer.h"

class GameState;

enum UpdateGroup : unsigned char
{
	EVERY_FRAME = 0,
	EVERY_LOAD = 1,
	EVERY_INIT = 2,
	ALL_GROUPS = 3
};

struct GameData
{
	virtual void update(GameState& game, bool alignToFourBytes = true) = 0;
	virtual void upload(GameState& game, uint32_t dataID, GameDataContainer* container = nullptr, uint32_t frameID = 0) = 0;

	void checkAndAllocate() {
		if (!data.size())
			data.resize(size);
	}

	uint32_t size;
	uint8_t updateGroup;

	std::vector<uint8_t> data;
};
