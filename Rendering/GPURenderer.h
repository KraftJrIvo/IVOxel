#pragma once

#include "IVoxelRenderer.h"

#include <VulkanRenderer.h>

class GPURenderer : public IVoxelRenderer
{
public:
	GPURenderer();
	virtual void render(const VoxelMap& map, Camera& cam);

private:
	VulkanRenderer _vulkanRenderer;
};