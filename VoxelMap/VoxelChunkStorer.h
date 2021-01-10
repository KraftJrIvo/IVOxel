#pragma once

#include "VoxelChunk.h"

class VoxelChunkStorer
{
public:
	VoxelChunkStorer() = default;
	VoxelChunkStorer(std::string worldDir);
	bool loadChunk(const std::vector<int32_t>& pos, VoxelChunk* chunk);
	void saveChunk(const VoxelChunk& chunk);
private:
	std::string _worldDir;
};