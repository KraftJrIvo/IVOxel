#include "GameDataMap.h"

#include "GameState.h"
#include <iostream>

GameDataMap::GameDataMap(uint32_t size_)
{
	size = max(size_, 64000);
	updateGroup = EVERY_LOAD;
}

void GameDataMap::update(GameState& game, uint32_t dataID, GameDataContainer* container, uint32_t frameID, bool alignToFourBytes)
{
	auto& cam = game.getCam();
	std::vector<int32_t> pos = { (int)floor(cam.pos.x), (int)floor(cam.pos.y), (int)floor(cam.pos.z) };
	auto mapData = game.getMap().getChunksData(game.getMap().getLoadRadius(), alignToFourBytes);

	checkAndAllocate();

	std::memcpy(data.data(), mapData.data(), min(mapData.size(), size));

	if (container) container->setData(dataID, data.data(), frameID);
}
