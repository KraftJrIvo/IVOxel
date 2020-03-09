#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanErrorChecker.h"

class VulkanInstance
{
public:
	VulkanInstance() = default;
	VulkanInstance(const VkInstanceCreateInfo& info);
	~VulkanInstance();

	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices();
	std::vector<VkPhysicalDevice> getAppropriatePhysicalDevices(const std::vector<VkPhysicalDevice>& physDevs, std::vector<uint32_t> queueFamilyFlags, VkPhysicalDeviceType type = VK_PHYSICAL_DEVICE_TYPE_OTHER);
	bool choosePhysicalDevice();

private:
	VkInstance _instance;
	VkPhysicalDevice _device;
	VkPhysicalDeviceProperties _deviceProps;
	VkPhysicalDeviceFeatures _deviceFeats;
	VkPhysicalDeviceMemoryProperties _deviceMemProps;
	VulkanErrorChecker _errorChecker;
};