#include "VulkanBuffer.h"

VulkanBuffer::VulkanBuffer(const VulkanDevice& device, void* ptr, uint32_t elemSz, uint32_t elemCnt) :
	_device(device)
{
	_ptr = ptr;
	_elemSz = elemSz;
	_elemCnt = elemCnt;
	_size = _elemSz * _elemCnt;
}

VulkanBuffer::~VulkanBuffer()
{
	auto& device = _device.getDevice();
	vkUnmapMemory(device, _memory);
	vkDestroyBuffer(device, _buffer, nullptr);
	vkFreeMemory(device, _memory, nullptr);
}

void VulkanBuffer::allocate()
{
	auto& device = _device.getDevice();

	VkBufferCreateInfo bufInfo = vkTypes::getBufCreateInfo(_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	
	auto res = vkCreateBuffer(device, &bufInfo, nullptr, &_buffer);

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(device, _buffer, &memReq);

	uint32_t memTypeId = _findMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memReq.memoryTypeBits);
	VkMemoryAllocateInfo memInfo = vkTypes::getMemAllocInfo(memReq.size, memTypeId);

	res = vkAllocateMemory(device, &memInfo, nullptr, &_memory);

	res = vkBindBufferMemory(device, _buffer, _memory, 0);

	res = vkMapMemory(device, _memory, 0, memReq.size, 0, &_mappedPtr);

	_allocated = true;
}

void VulkanBuffer::setData(uint32_t start, uint32_t count)
{
	memcpy((char*)_mappedPtr, (char*)_ptr, _elemSz * count);
}

uint32_t VulkanBuffer::_findMemoryType(VkMemoryPropertyFlags props, uint32_t typeFilter)
{
	auto devProps = _device.getPhysicalDevice()->getMemProps();
	uint32_t sz = devProps.memoryTypeCount;
	for (int i = 0; i < sz; ++i)
	{
		bool filterCondition = typeFilter & (1 << i);
		bool propCondition = ((devProps.memoryTypes[i].propertyFlags & props) == props);
		if (filterCondition && propCondition)
			return i;
	}

	return -1;
}
