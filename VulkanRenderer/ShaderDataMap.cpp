#include "ShaderDataMap.h"

#include "VulkanDescriptorPool.h"
#include "GameState.h"

void ShaderDataMap::update(VulkanDescriptorPool& pool, uint32_t frameID, uint32_t dataID, GameState& game)
{
	int32_t curOffset = 0;
	for (int i = -1; i <= 1; ++i)
		for (int j = -1; j <= 1; ++j)
			for (int k = -1; k <= 1; ++k)
			{
				uint32_t id = (k + 1) * 9 + (j + 1) * 3 + (i + 1);
				uint32_t vecId = id / 4;
				uint32_t elemId = id % 4;
				int32_t test = 100 * (k + 1) + 10 * (j + 1) + (i + 1);
				VoxelChunk* chunk = game.getMap().getChunk({ i,j,k });
				if (chunk)
				{
					std::memcpy((char*)state.chunkData + curOffset, chunk->pyramid.data.data(), chunk->pyramid.data.size());
					state.chunkOffsets[vecId][elemId] = curOffset;
					curOffset += chunk->pyramid.data.size();
				}
				else
				{
					state.chunkOffsets[vecId][elemId] = -1;
				}
			}

	pool.setData(frameID, dataID, &state);
}
