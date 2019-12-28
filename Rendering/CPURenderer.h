#pragma once

#include "IVoxelRenderer.h"

class CPURenderer
{
public:
	CPURenderer();
	virtual void render(const VoxelMap& map, const Camera& cam);
};