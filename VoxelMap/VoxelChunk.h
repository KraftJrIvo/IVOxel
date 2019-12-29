#pragma once

#include "VoxelPyramid.h"

struct VoxelChunk
{
	VoxelChunk();
	VoxelChunk(const std::vector<uint32_t>& size, const VoxelMapType& _type);

	// these are initially filled while loading map according to the map type
	std::vector<uint32_t> size;
	std::vector<uint8_t> vTypes;
	std::vector<uint8_t> vColors;
	std::vector<uint8_t> vNeighbours;

	VoxelMapType type;
	VoxelPyramid pyramid;

	void buildPyramid();
};