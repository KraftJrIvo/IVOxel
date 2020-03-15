#pragma once

#include <VulkanRenderer.h>

#include "IVoxelRenderer.h"

class GPURenderer : public IVoxelRenderer
{
public:
	GPURenderer();
	virtual void render(const VoxelMap& map, Camera& cam);

private:
	VulkanRenderer _vulkanRenderer;
};