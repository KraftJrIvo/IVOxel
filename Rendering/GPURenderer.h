#pragma once

#include "IVoxelRenderer.h"

#include "VulkanInstance.h"
#include "VulkanTypes.h"

class GPURenderer : public IVoxelRenderer
{
public:
	GPURenderer();
	virtual void render(const VoxelMap& map, Camera& cam) const;

private:
	VkInstance _inst;
	VulkanInstance _vulkan;
	std::vector<const char*> _layers;
	std::vector<const char*> _extensions;
};