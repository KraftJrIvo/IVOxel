#include "VoxelChunkStorer.h"

VoxelChunkStorer::VoxelChunkStorer(std::string worldDir) :
	_worldDir(worldDir)
{ }

bool VoxelChunkStorer::loadChunk(const std::vector<int32_t>&pos, VoxelChunk * chunk)
{
	return false;
}

void VoxelChunkStorer::saveChunk(const VoxelChunk& chunk)
{
	return;
}
