#include "GameDataLight.h"

#include "GameState.h"

GameDataLight::GameDataLight(uint8_t maxLights)
{
	updateGroup = EVERY_FRAME;
	size = maxLights * 8 * sizeof(float);
}

void GameDataLight::update(GameState& game, bool alignToFourBytes)
{
}

void GameDataLight::upload(GameState& game, uint32_t dataID, GameDataContainer* container, uint32_t frameID)
{
	if (container) container->setData(dataID, (void*)game.getLightData().data(), frameID);
}
