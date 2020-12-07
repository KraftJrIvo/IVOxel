#include "VulkanInstance.h"

VulkanInstance::VulkanInstance(std::vector<const char*> layers, std::vector<const char*> extensions)
{
	VkApplicationInfo app_info = vkTypes::getAppInfo();
	VkInstanceCreateInfo info = vkTypes::getInstanceCreateInfo(app_info, layers, extensions);

	vkCreateInstance(&info, nullptr, &_instance);
}

VulkanInstance::~VulkanInstance()
{
	if (_instance)
	{
		if (_deviceChosen)
		{
			_device.destroyPools();
			vkDestroyDevice(_device.get(), nullptr);
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

const VkInstance& VulkanInstance::getInstance()
{
	return _instance;
}

VulkanDevice& VulkanInstance::getDevice()
{
	return _device;
}

VulkanPhysicalDevice& VulkanInstance::getPhysDevice()
{
	return _physDevice;
}

VkPhysicalDevice VulkanInstance::getFirstAppropriatePhysicalDevice(const std::vector<VkPhysicalDevice>& physDevs, 
	std::vector<uint32_t> queueFamilyFlags, const VkSurfaceKHR& surface, VkPhysicalDeviceType type)
{
	std::vector<VkPhysicalDevice> appropriateDevices;

	for (auto& device : physDevs)
	{
		bool ok = true;
		std::vector<uint32_t> ids;
		for (auto& flag : queueFamilyFlags)
		{
			ids = VulkanPhysicalDevice::getQueueFamilyIndices(device, { flag });
			if (!ids.size())
			{
				ok = false;
				break;
			}
		}

		if (ok)
		{
			VkBool32 supportsWSI;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, ids[0], surface, &supportsWSI);
			if (supportsWSI)
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
	}

	if (appropriateDevices.size())
		return appropriateDevices[0];
	return VK_NULL_HANDLE;
}

bool VulkanInstance::chooseDevice(const std::vector<uint32_t>& queueFamilies, const std::vector<VkPhysicalDeviceType>& typesByPriority, const VkSurfaceKHR& surface)
{
	auto devices = getAvailablePhysicalDevices();

	int i = 0;
	VkPhysicalDevice appDevice = VK_NULL_HANDLE;
	while (appDevice == VK_NULL_HANDLE)
		appDevice = getFirstAppropriatePhysicalDevice(devices, queueFamilies, surface, typesByPriority[i++]);

	if (appDevice != VK_NULL_HANDLE)
	{
		_deviceChosen = true;

		_physDevice = VulkanPhysicalDevice(appDevice, queueFamilies);

		_device = VulkanDevice(_physDevice, {}, {VK_KHR_SWAPCHAIN_EXTENSION_NAME});

		return true;
	}

	return false;
}