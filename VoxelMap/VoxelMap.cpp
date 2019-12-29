#include "VoxelMap.h"

VoxelMap::VoxelMap()
{
}

VoxelMap::VoxelMap(const VoxelMapType& type) :
	_type(type)
{
}

void VoxelMap::buildPyramid()
{
	for (auto& chunk : _chunks)
		chunk.buildPyramid();
}