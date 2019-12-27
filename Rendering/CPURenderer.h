#pragma once

#include "IVoxelRenderer.h"

class CPURenderer
{
public:
	CPURenderer();
	virtual int render(const VoxelMap& map, const Camera& cam);
};