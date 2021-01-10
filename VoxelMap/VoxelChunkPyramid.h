#pragma once

#include "Voxel.h"

struct VoxelChunkPyramid
{
	VoxelChunkPyramid();
	VoxelChunkPyramid(const VoxelFormat& format_, uint32_t side, bool alignToFourBytes = true);

	uint8_t base;
	uint8_t power;
	uint32_t side;
	uint32_t nVoxBytes;

	bool alignToFourBytes;

	VoxelFormat format;
	std::vector<uint8_t> data;

	void build(const std::vector<Voxel>& voxels);

	static uint8_t getPyramLayerBytesCount(uint8_t base, uint8_t power);
};
