#pragma once

#include <VoxelMap.h>

#include "Camera.h"

class IVoxelRenderer
{
public:
	virtual void render(const VoxelMap& map, Camera& cam) = 0;
};