#include "VulkanDevice.h"
#include "VulkanErrorChecker.h"

VulkanDevice::VulkanDevice(const VulkanPhysicalDevice& physDev, const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	std::vector<VkDeviceQueueCreateInfo> queInfos;

	auto families = physDev.getQueueFamilyIndices(physDev.getDevice(), );
	std::vector<float> priorities(families.size());
	for (auto& fam : families)
		queInfos.push_back(vkTypes::getQFCreateInfo(families.size(), fam, priorities.data()));

	auto deviceCreateInfo = vkTypes::getDeviceCreateInfo(queInfos, &physDev.getFeats(), layers, extensions);

	vkCreateDevice(physDev.getDevice(), &deviceCreateInfo, nullptr, &_device);

	for (auto& fam : families)
	{
		vkGetDeviceQueue(_device, fam, 0, &_queues[fam]);
		auto info = vkTypes::getCPCreateInfo(fam);
		vkCreateCommandPool(_device, &info, nullptr, &_pools[fam]);
	}
}

const VkDevice& VulkanDevice::getDevice()
{
	return _device;
}

void VulkanDevice::destroyPools()
{
	for (auto& pool : _pools)
		vkDestroyCommandPool(_device, pool.second, nullptr);
}

void VulkanDevice::getCommand(VkCommandBuffer* bufs, uint32_t count, uint32_t queueFamId)
{
	auto info = vkTypes::getCBAllocateInfo(_pools[queueFamId], count);
	vkAllocateCommandBuffers(_device, &info, bufs);
}

void VulkanDevice::freeCommand(VkCommandBuffer* bufs, uint32_t count, uint32_t queueFamId)
{
	vkFreeCommandBuffers(_device, _pools[queueFamId], count, bufs);
}
