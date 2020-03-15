#pragma once

#include "VulkanInstance.h"

class VulkanRenderer
{
public:
	VulkanRenderer();
	
	void init();
	void run();
	void stop();
	bool runOnce();


private:
	VulkanInstance _vulkan;

	void _outputSupportedInstanceLayers();
	void _outputSupportedInstanceExtensions(const char* layerName);
	void _outputSupportedDeviceLayers();
	void _outputSupportedDeviceExtensions();

	bool _keepGoing = true;
};