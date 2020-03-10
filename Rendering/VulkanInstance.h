#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanDevice.h"

class VulkanInstance
{
public:
	VulkanInstance();
	~VulkanInstance();

	bool chooseDevice();
	VkPhysicalDevice getFirstAppropriatePhysicalDevice(const std::vector<VkPhysicalDevice>& physDevs, std::vector<uint32_t> queueFamilyFlags, VkPhysicalDeviceType type = VK_PHYSICAL_DEVICE_TYPE_OTHER);
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices();

private:
	std::vector<const char*> _layers;
	std::vector<const char*> _extensions;
	VkInstance _instance;
	VulkanDevice _device;
	VulkanPhysicalDevice _physDevice;
	bool _deviceChosen = false;
};