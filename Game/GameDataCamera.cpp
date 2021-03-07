#include "GameDataCamera.h"

#include "GameState.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

void GameDataCamera::update(GameDataContainer* container, uint32_t frameID, uint32_t dataID, GameState& game, bool alignToFourBytes)
{
	game.updateRot();
	game.updateTrans();

	auto& cam = game.getCam();
	container->setData(frameID, dataID, &cam);
}
