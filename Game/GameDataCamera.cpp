#include "GameDataCamera.h"

#include "GameState.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

GameDataCamera::GameDataCamera()
{
	size = sizeof(Camera);
	updateGroup = EVERY_FRAME;
}

void GameDataCamera::update(GameState& game, uint32_t dataID, GameDataContainer* container, uint32_t frameID, bool alignToFourBytes)
{
	auto& cam = game.getCam();

	checkAndAllocate();

	std::memcpy(data.data(), &cam, size);

	if (container) container->setData(dataID, data.data(), frameID);
}
