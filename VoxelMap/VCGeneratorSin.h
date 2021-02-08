#pragma once

#include "VoxelChunkGenerator.h"

class VCGenerator : public VoxelChunkGenerator
{
	VoxelChunk generateChunk(const VoxelMapFormat& format, uint32_t side, const std::vector<int32_t>& pos) const override;
	std::vector<Light> generateLights(const std::vector<int32_t>& pos, float radius, float time = 0.0f) const override;
};