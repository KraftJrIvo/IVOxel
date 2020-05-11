#include "GPURenderer.h"

#include "VulkanBuffer.h"

#include <iostream>

GPURenderer::GPURenderer()
{
	_vulkanRenderer.init();
}

void GPURenderer::render(const VoxelMap& map, Camera& cam)
{
	_vulkanRenderer.run(map);
}
