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
	VulkanInstance _vulkan;
	
	void _outputSupportedInstanceLayers();
	void _outputSupportedInstanceExtensions(const char* layerName);
	void _outputSupportedDeviceLayers();
	void _outputSupportedDeviceExtensions();

};