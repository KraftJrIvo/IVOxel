#include "VoxelMap.h"

VoxelMap::VoxelMap()
{
}

VoxelMap::VoxelMap(const VoxelMapType& type) :
	_type(type)
{
}

void VoxelMap::buildPyramid(uint16_t power)
{
	for (auto& chunk : _chunks)
		chunk.buildPyramid(power);
}

void VoxelMap::optimizePyramid()
{
	for (auto& chunk : _chunks)
		chunk.optimizePyramid();
}
