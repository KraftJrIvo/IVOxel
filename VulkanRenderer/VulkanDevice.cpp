#include "VulkanDevice.h"
#include "VulkanErrorChecker.h"

VulkanDevice::VulkanDevice(const VulkanPhysicalDevice& physDev, std::vector<const char*> layers, std::vector<const char*> extensions)
{
	_physDev = &physDev;

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
		auto info = vkTypes::getCPCreateInfo(id.first, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		vkCreateCommandPool(_device, &info, nullptr, &_pools[id.first]);
	}
}

const VkDevice& VulkanDevice::getDevice() const
{
	return _device;
}

const VulkanPhysicalDevice* VulkanDevice::getPhysicalDevice() const
{
	return _physDev;
}

void VulkanDevice::destroyPools()
{
	for (auto& pool : _pools)
		vkDestroyCommandPool(_device, pool.second, nullptr);
}

uint32_t VulkanDevice::getQFIdByType(uint32_t type) const
{
	if (_idsByType.count(type))
		return _idsByType.find(type)->second;
	return -1;
}

VkQueue VulkanDevice::getQueueById(uint32_t id) const
{
	if (_queues.count(id))
		return _queues.find(id)->second;
	return VK_NULL_HANDLE;
}

VkQueue VulkanDevice::getQueueByType(uint32_t type) const
{
	auto id = getQFIdByType(type);
	return getQueueById(id);
}

void VulkanDevice::getCommand(VkCommandBuffer* bufs, uint32_t count, uint32_t queueFamId) const
{
	auto info = vkTypes::getCBAllocateInfo(_pools.find(queueFamId)->second, count);
	vkAllocateCommandBuffers(_device, &info, bufs);
}

void VulkanDevice::freeCommand(VkCommandBuffer* bufs, uint32_t count, uint32_t queueFamId) const
{
	vkFreeCommandBuffers(_device, _pools.find(queueFamId)->second, count, bufs);
}
