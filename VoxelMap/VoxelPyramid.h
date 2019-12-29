#pragma once

#include "VoxelMapType.h"

struct VoxelPyramid
{
	VoxelPyramid();
	VoxelPyramid(const std::vector<uint32_t>& _size, const VoxelMapType& _type);

	uint32_t base;
	uint32_t power;
	uint32_t side;
	uint32_t nVoxBytes;

	VoxelMapType type;
	std::vector<std::vector<uint8_t>> data;

	void build(const std::vector<uint32_t>& size, const std::vector<uint8_t>& types, const std::vector<uint8_t>& colors, const std::vector<uint8_t>& neighbours);
};
