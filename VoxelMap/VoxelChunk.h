#pragma once

#include <vector>

#include "types.h"
#include "VoxelMapType.h"

struct VoxelChunk
{
	VoxelChunk();
	VoxelChunk(const std::vector<uint32_t>& size, const VoxelMapType& type);

	std::vector<std::vector<uint8_t>> vNLeavesOnLayers;
	std::vector<std::vector<uint8_t>> vLayerSizes;
	std::vector<std::vector<uint8_t>> vVoxPyramidOffsets;
	std::vector<std::vector<uint8_t>> vVoxPyramid;

	std::vector<uint32_t> size;
	std::vector<uint8_t> vTypes;
	std::vector<uint8_t> vColors;

	void buildPyramid(uint16_t power = -1);
	void optimizePyramid();
	void changeVoxel(const std::vector<uint32_t>& pos, uint8_t type, const std::vector<uint8_t>& color);

private:
	uint8_t _pyramidPower;
	uint8_t _nTypeBytes;
};