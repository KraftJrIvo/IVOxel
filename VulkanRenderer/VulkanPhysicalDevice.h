#pragma once

#include "VulkanTypes.h"
#include <map>

class VulkanPhysicalDevice
{
public:
	VulkanPhysicalDevice() = default;
	VulkanPhysicalDevice(const VkPhysicalDevice& dev, const std::vector<uint32_t>& queFamFlags);

	const VkPhysicalDevice& get() const;
	const VkPhysicalDeviceProperties& getProps() const;
	const VkPhysicalDeviceMemoryProperties& getMemProps() const;
	const VkPhysicalDeviceFeatures& getFeats() const;
	const std::vector<uint32_t>& getQueueTypes() const;
	const std::vector<uint32_t>& getQueueIndicesByType(uint32_t type) const;

	static std::vector<uint32_t> getQueueFamilyIndices(const VkPhysicalDevice& dev, std::vector<uint32_t> queueFamilyFlags);

private:
	VkPhysicalDevice _device;
	VkPhysicalDeviceProperties _deviceProps;
	VkPhysicalDeviceFeatures _deviceFeats;
	VkPhysicalDeviceMemoryProperties _deviceMemProps;
	std::vector<uint32_t> _queTypes;
	std::map<uint32_t, std::vector<uint32_t>> _queFamIndicesByType;
};