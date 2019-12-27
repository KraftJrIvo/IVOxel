#pragma once

#include <vector>

#include "types.h"

#define PYRAMID_BASE 2

struct VoxelChunk
{
	VoxelChunk();
	VoxelChunk(const std::vector<int>& size, VoxelMapType type);

	std::vector<std::vector<unsigned char>> vNLeavesOnLayers;
	std::vector<std::vector<unsigned char>> vLayerSizes;
	std::vector<std::vector<unsigned char>> vVoxPyramidOffsets;
	std::vector<std::vector<unsigned char>> vVoxPyramid;

	std::vector<int> size;
	std::vector<unsigned char> vTypes;
	std::vector<unsigned char> vColors;

private:
	unsigned char _nPyramidPower;
	unsigned char _nTypeBytes;
};