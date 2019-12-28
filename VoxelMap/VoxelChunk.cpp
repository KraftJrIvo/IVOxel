#include "VoxelChunk.h"

#include <algorithm>

VoxelChunk::VoxelChunk() :
	_pyramidPower(0),
	_nTypeBytes(1)
{
}

VoxelChunk::VoxelChunk(const std::vector<uint32_t>& size, const VoxelMapType& type)
{
	uint32_t maxDim = std::max(size[0], std::max(size[1], size[2]));
	_pyramidPower = uint8_t(std::ceil(std::log2(maxDim)));

	_nTypeBytes = type.sizeInBytes;
}

void VoxelChunk::buildPyramid(uint16_t power)
{
	if (power != -1)
		_pyramidPower = power;

	//std::vector
}

void VoxelChunk::optimizePyramid()
{
}

void VoxelChunk::changeVoxel(const std::vector<uint32_t>& pos, uint8_t type, const std::vector<uint8_t>& color)
{
}
