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

	int i = 0;
	_queues.resize(families.size());
	for (auto& fam : families)
		vkGetDeviceQueue(_device, fam, 0, &_queues[i++]);
}

const VkDevice& VulkanDevice::getDevice()
{
	return _device;
}
