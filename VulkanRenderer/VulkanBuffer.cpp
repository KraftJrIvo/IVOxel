#include "VulkanBuffer.h"

VulkanBuffer::~VulkanBuffer()
{
	if (_allocated)
	{
		auto& device = _dev->getDevice();
		vkDestroyBuffer(device, _buffer, nullptr);
		vkFreeMemory(device, _memory, nullptr);
	}
}

void VulkanBuffer::create(const VulkanDevice& dev, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlags usage, VkMemoryPropertyFlags props)
{
	_elemSz = elemSz;
	_nElems = nElems;
	_size = _elemSz * _nElems;
	_dev = &dev;
	auto& device = dev.getDevice();

	VkBufferCreateInfo bufferInfo = vkTypes::getBufCreateInfo(_size, usage);

	vkCreateBuffer(device, &bufferInfo, nullptr, &_buffer);

	vkGetBufferMemoryRequirements(device, _buffer, &_memReqs);

	VkMemoryAllocateInfo allocInfo = vkTypes::getMemAllocInfo(_memReqs.size, _findMemoryType(props, _memReqs.memoryTypeBits));

	vkAllocateMemory(device, &allocInfo, nullptr, &_memory);

	vkBindBufferMemory(device, _buffer, _memory, 0);

	_allocated = true;
}

void VulkanBuffer::setData(void* ptr, uint32_t start, uint32_t count)
{
	vkMapMemory(_dev->getDevice(), _memory, 0, _memReqs.size, 0, &_mappedPtr);
	memcpy((char*)_mappedPtr, (char*)ptr, _elemSz * count);
	vkUnmapMemory(_dev->getDevice(), _memory);
}

void VulkanBuffer::copyTo(VulkanBuffer& to)
{
	VkCommandBuffer commandBuf;
	_dev->getCommand(&commandBuf, 1, _dev->getQFIdByType(VK_QUEUE_TRANSFER_BIT));
	
	auto beginInfo = vkTypes::getCBBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	vkBeginCommandBuffer(commandBuf, &beginInfo);

	VkBufferCopy copyRegion = {0, 0, _size};
	vkCmdCopyBuffer(commandBuf, _buffer, to.getBuffer(), 1, &copyRegion);

	vkEndCommandBuffer(commandBuf);

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	std::vector<VkCommandBuffer> commandBufs = {commandBuf};
	VkSubmitInfo submitInfo = vkTypes::getSubmitInfo({}, {}, commandBufs, nullptr);

	auto transferQueue = _dev->getQueueByType(VK_QUEUE_TRANSFER_BIT);
	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);

	_dev->freeCommand(&commandBuf, 1, _dev->getQFIdByType(VK_QUEUE_TRANSFER_BIT));
}

const VkBuffer& VulkanBuffer::getBuffer()
{
	return _buffer;
}

uint32_t VulkanBuffer::getElemsCount()
{
	return _nElems;
}

uint32_t VulkanBuffer::_findMemoryType(VkMemoryPropertyFlags props, uint32_t typeFilter)
{
	auto devProps = _dev->getPhysicalDevice()->getMemProps();
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
