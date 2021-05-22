#include "GameDataLight.h"

#include "GameState.h"

GameDataLight::GameDataLight(uint8_t maxLights)
{
	updateGroup = EVERY_FRAME;
	size = maxLights * 8 * sizeof(float);
}

void GameDataLight::update(GameState& game, uint32_t dataID, GameDataContainer* container, uint32_t frameID, bool alignToFourBytes)
{
	auto& cam = game.getCam();
	std::vector<int32_t> pos = { (int)floor(cam.pos.x), (int)floor(cam.pos.y), (int)floor(cam.pos.z) };
	auto lightData = game.getMap().getLightDataAt(pos, game.getMap().getLoadRadius(), game.getTime());

	checkAndAllocate();

	std::memcpy(data.data(), lightData.data(), min(lightData.size(), size));

	if (container) container->setData(dataID, data.data(), frameID);
}
