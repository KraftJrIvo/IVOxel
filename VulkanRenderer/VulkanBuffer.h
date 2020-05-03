#pragma once

#include "VulkanTypes.h"
#include "VulkanDevice.h"

class VulkanBuffer
{
public:
	VulkanBuffer() = default;
	~VulkanBuffer();

	void create(const VulkanDevice& dev, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
	void setData(void* ptr, uint32_t start, uint32_t count);
	void copyTo(VulkanBuffer& to);

	const VkBuffer& getBuffer();
	uint32_t getElemsCount();

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

	uint32_t _findMemoryType(VkMemoryPropertyFlags props, uint32_t typeFilter);
};