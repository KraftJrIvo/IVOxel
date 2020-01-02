#pragma once

#include <VoxelMap.h>

#include "Camera.h"

class IVoxelRenderer
{
public:
	virtual void render(const VoxelMap& map, const Camera& cam) = 0;
};