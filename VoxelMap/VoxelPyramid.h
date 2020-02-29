#pragma once

#include "VoxelMapType.h"

struct VoxelPyramid
{
	VoxelPyramid();
	VoxelPyramid(const std::vector<uint32_t>& _size, const VoxelMapType& _type);

	uint8_t base;
	uint8_t power;
	uint32_t side;
	uint32_t nVoxBytes;

	VoxelMapType type;
	std::vector<uint8_t> data;

	void build(const std::vector<uint32_t>& size, const std::vector<uint8_t>& types, const std::vector<uint8_t>& colors, const std::vector<uint8_t>& neighbours);

	static uint8_t getPyramLayerBytesCount(uint8_t base, uint8_t power);
};
