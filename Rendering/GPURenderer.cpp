#include "GPURenderer.h"

GPURenderer::GPURenderer()
{
	_vulkan.chooseDevice();
}

void GPURenderer::render(const VoxelMap& map, Camera& cam) const
{
}
