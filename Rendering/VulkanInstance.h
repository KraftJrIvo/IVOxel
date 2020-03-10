#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanErrorChecker.h"
#include "VulkanPhysicalDevice.h"

class VulkanInstance
{
public:
	VulkanInstance() = default;
	VulkanInstance(const VkInstanceCreateInfo& info);
	~VulkanInstance();

	bool choosePhysicalDevice();
	VkPhysicalDevice getFirstAppropriatePhysicalDevice(const std::vector<VkPhysicalDevice>& physDevs, std::vector<uint32_t> queueFamilyFlags, VkPhysicalDeviceType type = VK_PHYSICAL_DEVICE_TYPE_OTHER);
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices();

private:
	VkInstance _instance;
	VulkanPhysicalDevice _device;
	VulkanErrorChecker _errorChecker;
};