#pragma once

#include "Camera.h"
#include <VoxelMap.h>

class IVoxelRenderer
{
public:
	virtual void render(const VoxelMap& map, const Camera& cam) = 0;
};