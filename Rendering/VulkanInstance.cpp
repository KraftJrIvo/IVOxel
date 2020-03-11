#include "VulkanInstance.h"

VulkanInstance::VulkanInstance()
{
	VkApplicationInfo app_info = vkTypes::getAppInfo();
	VkInstanceCreateInfo info = vkTypes::getInstanceCreateInfo(app_info, _layers, _extensions);

	vkCreateInstance(&info, nullptr, &_instance);
}

VulkanInstance::~VulkanInstance()
{
	if (_instance)
	{
		if (_deviceChosen)
		{
			_device.destroyPools();
			vkDestroyDevice(_device.getDevice(), nullptr);
		}
		vkDestroyInstance(_instance, nullptr);
	}
}

std::vector<VkPhysicalDevice>VulkanInstance::getAvailablePhysicalDevices()
{
	uint32_t nDevices = 0;
	vkEnumeratePhysicalDevices(_instance, &nDevices, nullptr);

	std::vector<VkPhysicalDevice> devices(nDevices);
	vkEnumeratePhysicalDevices(_instance, &nDevices, devices.data());

	return devices;
}

VulkanDevice& VulkanInstance::getDevice()
{
	return _device;
}

VulkanPhysicalDevice& VulkanInstance::getPhysDevice()
{
	return _physDevice;
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

bool VulkanInstance::chooseDevice(const std::vector<uint32_t>& queueFamilies, const std::vector<VkPhysicalDeviceType>& typesByPriority)
{
	_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	//_extensions.push_back("VK_EXT_debug_report");
	
	auto devices = getAvailablePhysicalDevices();

	int i = 0;
	VkPhysicalDevice appDevice = VK_NULL_HANDLE;
	while (appDevice == VK_NULL_HANDLE)
		appDevice = getFirstAppropriatePhysicalDevice(devices, queueFamilies, typesByPriority[i]);

	if (appDevice != VK_NULL_HANDLE)
	{
		_deviceChosen = true;

		_physDevice = VulkanPhysicalDevice(appDevice, queueFamilies);

		_device = VulkanDevice(_physDevice, _layers, _extensions);

		return true;
	}

	return false;
}