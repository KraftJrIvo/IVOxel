#pragma once

#include "IVoxelRenderer.h"

class GPURenderer
{
public:
	GPURenderer();
	virtual int render(const VoxelMap& map);
};