#include "GameDataConst.h"

#include <types.hpp>

#include <vector>

GameDataConst::GameDataConst(uint8_t chunkLoadRadius, uint8_t maxLights, float epsilon) :
	_chunkLoadRadius(chunkLoadRadius),
	_maxLights(maxLights),
	_epsilon(epsilon)
{
    size = 3 * sizeof(float);
    updateGroup = EVERY_INIT;
}

void GameDataConst::update(GameState& game, uint32_t dataID, GameDataContainer* container, uint32_t frameID, bool alignToFourBytes)
{
	std::vector<uint8_t> vdata;
	utils::appendBytes(vdata, (float)_chunkLoadRadius);
	utils::appendBytes(vdata, (float)_maxLights);
	utils::appendBytes(vdata, _epsilon);

	checkAndAllocate();

	std::memcpy(data.data(), vdata.data(), size);

	if (container) container->setData(dataID, data.data(), frameID);
}
