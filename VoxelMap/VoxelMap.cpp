#include "VoxelMap.h"

VoxelMap::VoxelMap() :
	_type(ONE_BYTE_RGB256)
{
}

VoxelMap::VoxelMap(VoxelMapType type) :
	_type(type)
{
}

int VoxelMap::optimize()
{
	return 0;
}
