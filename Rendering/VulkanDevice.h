#pragma once

#include "VulkanTypes.h"
#include "VulkanPhysicalDevice.h"

class VulkanDevice
{
public:
	VulkanDevice() = default;
	VulkanDevice(const VulkanPhysicalDevice& physDev, const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

	const VkDevice& getDevice();

private:
	VkDevice _device;
	std::vector<VkQueue> _queues;
};