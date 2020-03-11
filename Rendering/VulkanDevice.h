#pragma once

#include "VulkanTypes.h"
#include "VulkanPhysicalDevice.h"

class VulkanDevice
{
public:
	VulkanDevice() = default;
	VulkanDevice(const VulkanPhysicalDevice& physDev, const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

	const VkDevice& getDevice();
	void destroyPools();

	void getCommand(VkCommandBuffer* bufs, uint32_t count, uint32_t queueFamId);
	void freeCommand(VkCommandBuffer* bufs, uint32_t count, uint32_t queueFamId);

private:
	VkDevice _device;
	std::map<uint32_t, VkQueue> _queues;
	std::map<uint32_t, VkCommandPool> _pools;
};