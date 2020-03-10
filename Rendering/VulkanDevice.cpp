#include "VulkanDevice.h"
#include "VulkanErrorChecker.h"

VulkanDevice::VulkanDevice(const VulkanPhysicalDevice& physDev, const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	std::vector<VkDeviceQueueCreateInfo> queInfos;

	float priority = 1.0f;
	auto families = physDev.getQueueFamilies();
	for (auto& fam : families)
		queInfos.push_back(vkTypes::getQFCreateInfo(fam, &priority));

	auto deviceCreateInfo = vkTypes::getDeviceCreateInfo(queInfos, &physDev.getFeats(), layers, extensions);

	vkCreateDevice(physDev.getDevice(), &deviceCreateInfo, nullptr, &_device);

	_queues.resize(families.size());
	int i = 0;
	for (auto& fam : families)
		vkGetDeviceQueue(_device, fam, i, &_queues[i++]);
}

const VkDevice& VulkanDevice::getDevice()
{
	return _device;
}
