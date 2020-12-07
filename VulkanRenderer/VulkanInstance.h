#pragma once

#include "VulkanDevice.h"

class VulkanInstance
{
public:
	VulkanInstance(std::vector<const char*> layers = {}, std::vector<const char*> extensions = {});
	~VulkanInstance();

	bool chooseDevice(const std::vector<uint32_t>& queueFamilies, const std::vector<VkPhysicalDeviceType>& typesByPriority, const VkSurfaceKHR& surface);
	VkPhysicalDevice getFirstAppropriatePhysicalDevice(const std::vector<VkPhysicalDevice>& physDevs, std::vector<uint32_t> queueFamilyFlags, 
		const VkSurfaceKHR& surface, VkPhysicalDeviceType type = VK_PHYSICAL_DEVICE_TYPE_OTHER);
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices();

	const VkInstance& getInstance();
	VulkanDevice& getDevice();
	VulkanPhysicalDevice& getPhysDevice();

private:
	VkInstance _instance;
	VulkanDevice _device;
	VulkanPhysicalDevice _physDevice;
	bool _deviceChosen = false;
};