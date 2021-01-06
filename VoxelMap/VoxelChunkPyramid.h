#pragma once

#include "VoxelMapFormat.h"
#include "Voxel.h"

struct VoxelChunkPyramid
{
	VoxelChunkPyramid();
	VoxelChunkPyramid(const VoxelChunkFormat& format_);

	uint8_t base;
	uint8_t power;
	uint32_t side;
	uint32_t nVoxBytes;

	VoxelChunkFormat format;
	std::vector<uint8_t> data;

	void build(const std::vector<Voxel>& voxels);

	static uint8_t getPyramLayerBytesCount(uint8_t base, uint8_t power);
};
