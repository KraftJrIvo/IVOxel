#pragma once

#include "VulkanTypes.h"
#include "VulkanPhysicalDevice.h"

class VulkanDevice
{
public:
	VulkanDevice() = default;
	VulkanDevice(const VulkanPhysicalDevice& physDev, std::vector<const char*> layers = {}, std::vector<const char*> extensions = {});

	const VkDevice& get() const;
	const VulkanPhysicalDevice* getPhysicalDevice() const;
	void destroyPools();
	uint32_t getQFIdByType(uint32_t type) const;
	VkQueue getQueueById(uint32_t id) const;
	VkQueue getQueueByType(uint32_t type) const;

	void getCommand(VkCommandBuffer* bufs, uint32_t count, uint32_t queueFamId) const;
	void freeCommand(VkCommandBuffer* bufs, uint32_t count, uint32_t queueFamId) const;

private:
	VkDevice _device;
	const VulkanPhysicalDevice* _physDev;
	std::map<uint32_t, bool> _uniqueIds;
	std::map<uint32_t, uint32_t> _idsByType;
	std::map<uint32_t, VkQueue> _queues;
	std::map<uint32_t, VkCommandPool> _pools;
};