#pragma once

#include "VoxelChunkGenerator.h"

class VCGenerator : public VoxelChunkGenerator
{
	VoxelChunk generate(const VoxelMapFormat& format, uint32_t side, const std::vector<int32_t>& pos) const override;
};