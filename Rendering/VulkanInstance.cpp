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

VkPhysicalDevice VulkanInstance::getFirstAppropriatePhysicalDevice(const std::vector<VkPhysicalDevice>& physDevs, std::vector<uint32_t> queueFamilyFlags, VkPhysicalDeviceType type)
{
	std::vector<VkPhysicalDevice> appropriateDevices;

	for (auto& device : physDevs)
	{
		bool ok = true;
		for (auto& flag : queueFamilyFlags)
		{
			auto ids = VulkanPhysicalDevice::getQueueFamilyIndices(device, { flag });
			if (!ids.size())
			{
				ok = false;
				break;
			}
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

	if (appropriateDevices.size())
		return appropriateDevices[0];
	return VK_NULL_HANDLE;
}

bool VulkanInstance::choosePhysicalDevice()
{
	auto devices = getAvailablePhysicalDevices();
	std::vector<uint32_t> queueFamilies = { VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT };
	auto appDevice = getFirstAppropriatePhysicalDevice(devices, queueFamilies, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
	if (appDevice == VK_NULL_HANDLE)
		appDevice = getFirstAppropriatePhysicalDevice(devices, queueFamilies, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);

	if (appDevice != VK_NULL_HANDLE)
	{
		_device = VulkanPhysicalDevice(appDevice, queueFamilies);

		std::cout << "Device was chosen: " << _device.getProps().deviceName << std::endl;
		return true;
	}

	std::cout << "No device was chosen." << std::endl;
	return false;
}