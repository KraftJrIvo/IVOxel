#pragma once

#include "VoxelChunk.h"

class VoxelMap
{
public:
	VoxelMap();
	virtual int save() = 0;
	virtual int load() = 0;

private:
	virtual int _ivoxelize() = 0;
};