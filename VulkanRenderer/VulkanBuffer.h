#pragma once

#include "VulkanTypes.h"
#include "VulkanDevice.h"

class VulkanBuffer
{
public:
	~VulkanBuffer();

	void create(const VulkanDevice& dev, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
	void setData(void* ptr, uint32_t start, uint32_t count);
	void copyTo(VulkanBuffer& to);

	const VkBuffer& getBuffer();
	uint32_t getElemsCount();

	void initHost(const VulkanDevice& device, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage);
	void initStaging(const VulkanDevice& device, void* data, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage);

	static uint32_t findMemoryType(const VulkanDevice& device, VkMemoryPropertyFlags props, uint32_t typeFilter);

private:
	VkBuffer _buffer;

	void* _mappedPtr;
	uint32_t _elemSz;
	uint32_t _nElems;
	VkDeviceSize _size;
	const VulkanDevice* _dev;
	VkDeviceMemory _memory;
	VkMemoryRequirements _memReqs;

	bool _allocated;
};