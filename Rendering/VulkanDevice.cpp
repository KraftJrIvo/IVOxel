#include "VulkanDevice.h"
#include "VulkanErrorChecker.h"

VulkanDevice::VulkanDevice(const VulkanPhysicalDevice& physDev, const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	std::vector<VkDeviceQueueCreateInfo> queInfos;

	auto types = physDev.getQueueTypes();
	std::vector<float> priorities(types.size());
	for (auto& type : types)
	{
		auto ids = physDev.getQueueIndicesByType(type);
		uint32_t id = ids[0];
		if (ids.size())
		{
			_uniqueIds[id] = true;
			_idsByType[type] = id;
		}
	}

	for (auto& id : _uniqueIds)
		queInfos.push_back(vkTypes::getQFCreateInfo(_uniqueIds.size(), id.first, priorities.data()));

	auto deviceCreateInfo = vkTypes::getDeviceCreateInfo(queInfos, &physDev.getFeats(), layers, extensions);

	vkCreateDevice(physDev.getDevice(), &deviceCreateInfo, nullptr, &_device);

	for (auto& id : _uniqueIds)
	{
		vkGetDeviceQueue(_device, id.first, 0, &_queues[id.first]);
		auto info = vkTypes::getCPCreateInfo(id.first);
		vkCreateCommandPool(_device, &info, nullptr, &_pools[id.first]);
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

uint32_t VulkanDevice::getQFIdByType(uint32_t type)
{
	if (_idsByType.count(type))
		return _idsByType.find(type)->second;
	return -1;
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
