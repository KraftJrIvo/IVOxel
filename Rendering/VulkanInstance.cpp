#include "VulkanInstance.h"

#include <iostream>

VulkanInstance::VulkanInstance(const VkInstanceCreateInfo& info)
{
	vkCreateInstance(&info, nullptr, &_instance);
}

VulkanInstance::~VulkanInstance()
{
	if (_instance)
		vkDestroyInstance(_instance, nullptr);
}

std::vector<VkPhysicalDevice>VulkanInstance::getAvailablePhysicalDevices()
{
	uint32_t nDevices = 0;
	vkEnumeratePhysicalDevices(_instance, &nDevices, nullptr);

	std::vector<VkPhysicalDevice> devices(nDevices);
	vkEnumeratePhysicalDevices(_instance, &nDevices, devices.data());

	return devices;
}

std::vector<VkPhysicalDevice> VulkanInstance::getAppropriatePhysicalDevices(const std::vector<VkPhysicalDevice>& physDevs, std::vector<uint32_t> queueFamilyFlags, VkPhysicalDeviceType type)
{
	std::vector<VkPhysicalDevice> appropriateDevices;

	for (auto& device : physDevs)
	{
		uint32_t nQueueFamilies;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &nQueueFamilies, nullptr);
		std::vector<VkQueueFamilyProperties> queueFams;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &nQueueFamilies, queueFams.data());

		bool ok = true;
		for (auto& queueFam : queueFams)
		{
			for (auto& flag : queueFamilyFlags)
				if (!(queueFam.queueFlags & flag))
				{
					ok = false;
					break;
				}
			if (!ok)
				break;
		}

		if (ok)
		{
			VkPhysicalDeviceProperties props;
			if (type != VK_PHYSICAL_DEVICE_TYPE_OTHER)
				vkGetPhysicalDeviceProperties(device, &props);
			else
				props.deviceType = type;

			if (props.deviceType == type)
				appropriateDevices.push_back(device);
		}
	}

	return appropriateDevices;
}

bool VulkanInstance::choosePhysicalDevice()
{
	auto devices = getAvailablePhysicalDevices();
	auto appDevices = getAppropriatePhysicalDevices(devices, { VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT }, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

	if (appDevices.size())
	{
		_device = appDevices[0];
		vkGetPhysicalDeviceProperties(_device, &_deviceProps);
		vkGetPhysicalDeviceFeatures(_device, &_deviceFeats);
		vkGetPhysicalDeviceMemoryProperties(_device, &_deviceMemProps);

		std::cout << "Device was chosen: " << _deviceProps.deviceName << std::endl;
		return true;
	}
	std::cout << "No device was chosen." << std::endl;
	return false;
}
