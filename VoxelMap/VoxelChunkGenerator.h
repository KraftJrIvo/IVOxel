#pragma once

#include "VoxelChunk.h"
#include "VoxelMapFormat.h"

class VoxelChunkGenerator
{
public:
	void setSeed(unsigned long long seed);
	virtual VoxelChunk generate(const VoxelMapFormat& format, uint32_t side, const std::vector<int32_t>& pos) const = 0;

private:
	unsigned long long _seed;
};