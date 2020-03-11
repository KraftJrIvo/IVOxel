#include "VulkanPhysicalDevice.h"

VulkanPhysicalDevice::VulkanPhysicalDevice(const VkPhysicalDevice& dev, const std::vector<uint32_t>& queFamFlags) :
	_device(dev)
{
	vkGetPhysicalDeviceProperties(_device, &_deviceProps);
	vkGetPhysicalDeviceFeatures(_device, &_deviceFeats);
	vkGetPhysicalDeviceMemoryProperties(_device, &_deviceMemProps);
	for (auto& flag : queFamFlags)
	{
		_queTypes.push_back(flag);
		_queFamIndicesByType[flag] = getQueueFamilyIndices(dev, { flag });
	}
}

const VkPhysicalDevice& VulkanPhysicalDevice::getDevice() const
{
	return _device;
}

const VkPhysicalDeviceProperties& VulkanPhysicalDevice::getProps() const
{
	return _deviceProps;
}

const VkPhysicalDeviceFeatures& VulkanPhysicalDevice::getFeats() const
{
	return _deviceFeats;
}

const std::vector<uint32_t>& VulkanPhysicalDevice::getQueueTypes() const
{
	return _queTypes;
}

const std::vector<uint32_t>& VulkanPhysicalDevice::getQueueIndicesByType(uint32_t type) const
{
	if (_queFamIndicesByType.count(type))
		return _queFamIndicesByType.find(type)->second;
	return {};
}

std::vector<uint32_t> VulkanPhysicalDevice::getQueueFamilyIndices(const VkPhysicalDevice& dev, std::vector<uint32_t> queueFamilyFlags)
{
	std::vector<uint32_t> indices;

	uint32_t nQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &nQueueFamilies, nullptr);
	std::vector<VkQueueFamilyProperties> queueFams(nQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &nQueueFamilies, queueFams.data());

	bool ok = true;
	int i = 0;
	for (auto& queueFam : queueFams)
	{
		for (auto& flag : queueFamilyFlags)
			if (!(queueFam.queueFlags & flag))
			{
				ok = false;
				break;
			}

		if (ok)
			indices.push_back(i++);
	}

	return indices;
}