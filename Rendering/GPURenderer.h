#pragma once

#include "IVoxelRenderer.h"

class GPURenderer : public IVoxelRenderer
{
public:
	GPURenderer();
	virtual void render(const VoxelMap& map, const Camera& cam);
};