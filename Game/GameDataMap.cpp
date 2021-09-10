#include "GameDataMap.h"

#include "GameState.h"
#include <iostream>

GameDataMap::GameDataMap(uint32_t size_)
{
	size = max(size_, 64000);
	updateGroup = EVERY_LOAD;
}

void GameDataMap::update(GameState& game, bool alignToFourBytes)
{
}

void GameDataMap::upload(GameState& game, uint32_t dataID, GameDataContainer* container, uint32_t frameID)
{
	if (container) container->setData(dataID, (void*)game.getMapData().data(), frameID);
}

