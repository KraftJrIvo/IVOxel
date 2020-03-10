#pragma once

#include "VulkanTypes.h"
#include <map>

class VulkanPhysicalDevice
{
public:
	VulkanPhysicalDevice() = default;
	VulkanPhysicalDevice(const VkPhysicalDevice& dev, const std::vector<uint32_t>& queFamFlags);

	VkPhysicalDeviceProperties getProps();

	static std::vector<uint32_t> getQueueFamilyIndices(const VkPhysicalDevice& dev, std::vector<uint32_t> queueFamilyFlags);

private:
	VkPhysicalDevice _device;
	VkPhysicalDeviceProperties _deviceProps;
	VkPhysicalDeviceFeatures _deviceFeats;
	VkPhysicalDeviceMemoryProperties _deviceMemProps;
	std::map<uint32_t, std::vector<uint32_t>> _queFamIndicesByFlag;
};