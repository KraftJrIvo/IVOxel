#pragma once

#include "Light.h"
#include "VoxelChunk.h"
#include "VoxelMapFormat.h"

class VoxelChunkGenerator
{
public:
	void setSeed(unsigned long long seed);
	virtual VoxelChunk generateChunk(const VoxelMapFormat& format, uint32_t side, const std::vector<int32_t>& pos) const = 0;
	virtual std::vector<Light> generateLights(const std::vector<int32_t>& pos, float radius, float time = 0.0f) const = 0;

private:
	unsigned long long _seed;
};