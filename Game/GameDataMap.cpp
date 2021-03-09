#include "GameDataMap.h"

#include "GameState.h"

void GameDataMap::update(GameDataContainer* container, uint32_t frameID, uint32_t dataID, GameState& game, bool alignToFourBytes)
{
	std::memset(&data, 0, sizeof(data));
	auto mapData = game.getMap()->getChunksDataAt({ (int32_t)game.getTrans().x, (int32_t)game.getTrans().y, (int32_t)game.getTrans().z }, r, alignToFourBytes);
	std::memcpy(&data, mapData.data(), mapData.size());
	container->setData(frameID, dataID, &data);
}
