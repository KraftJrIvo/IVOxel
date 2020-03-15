#pragma once

#include "VulkanInstance.h"

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();
	
	void init();
	void run();
	void stop();
	bool runOnce();


private:
	VulkanInstance _vulkan;
	VkSurfaceKHR _surface;
	VkSurfaceFormatKHR _surfaceFormat;

	void _initSurface();
	void _setSurfaceFormat(const VulkanDevice&);
	void _outputSupportedInstanceLayers();
	void _outputSupportedInstanceExtensions(const char* layerName);
	void _outputSupportedDeviceLayers();
	void _outputSupportedDeviceExtensions();

	bool _keepGoing = true;
};