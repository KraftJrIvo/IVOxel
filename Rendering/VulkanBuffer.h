#pragma once

#include "VulkanTypes.h"
#include "VulkanDevice.h"

class VulkanBuffer
{
public:
	VulkanBuffer(const VulkanDevice& device, void* ptr, uint32_t elemSz, uint32_t elemCnt);
	~VulkanBuffer();

	void allocate();

	void setData(uint32_t start, uint32_t count);

private:
	VkBuffer _buffer;

	void* _ptr;
	void* _mappedPtr;
	uint32_t _elemSz;
	uint32_t _elemCnt;
	VkDeviceSize _size;

	const VulkanDevice& _device;
	VkDeviceMemory _memory;

	bool _allocated;

	uint32_t _findMemoryType(VkMemoryPropertyFlags props, uint32_t typeFilter);
};