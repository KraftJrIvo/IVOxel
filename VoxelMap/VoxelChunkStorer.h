#pragma once

#include "VoxelChunk.h"

class VoxelChunkStorer
{
public:
	VoxelChunkStorer() = default;
	VoxelChunkStorer(std::string worldDir);
	VoxelChunk loadChunk(const std::vector<int32_t>& pos);
	VoxelChunk saveChunk(const VoxelChunk& chunk);
private:
};