#include "GameDataLight.h"

#include "GameState.h"

void GameDataLight::update(GameDataContainer* container, uint32_t frameID, uint32_t dataID, GameState& game, bool alignToFourBytes)
{
	std::memset(&data, 0, sizeof(data));
	auto mapLightData = game.getMap()->getLightDataAt({ (int32_t)game.getTrans().x, (int32_t)game.getTrans().y, (int32_t)game.getTrans().z }, r);
	std::memcpy(&data, mapLightData.data(), mapLightData.size());
	if (container) container->setData(frameID, dataID, &data);
}
