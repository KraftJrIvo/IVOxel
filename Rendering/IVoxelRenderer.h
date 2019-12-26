#pragma once

#include <VoxelMap.h>

class IVoxelRenderer
{
public:
	virtual int render(const VoxelMap& map) = 0;
};