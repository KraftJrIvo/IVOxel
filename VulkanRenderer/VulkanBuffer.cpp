#include "VulkanBuffer.h"

VulkanBuffer::~VulkanBuffer()
{
	if (_allocated)
	{
		auto& device = _dev->get();
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
	auto& device = dev.get();

	VkBufferCreateInfo bufferInfo = vkTypes::getBufCreateInfo(_size, usage);

	vkCreateBuffer(device, &bufferInfo, nullptr, &_buffer);

	vkGetBufferMemoryRequirements(device, _buffer, &_memReqs);

	VkMemoryAllocateInfo allocInfo = vkTypes::getMemAllocInfo(_memReqs.size, findMemoryType(*_dev, props, _memReqs.memoryTypeBits));

	vkAllocateMemory(device, &allocInfo, nullptr, &_memory);

	vkBindBufferMemory(device, _buffer, _memory, 0);

	_allocated = true;
}

void VulkanBuffer::setData(void* ptr, uint32_t start, uint32_t size)
{
	vkMapMemory(_dev->get(), _memory, 0, _memReqs.size, 0, &_mappedPtr);
	memcpy((char*)_mappedPtr + start, (char*)ptr, size);
	vkUnmapMemory(_dev->get(), _memory);
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

void VulkanBuffer::initHost(const VulkanDevice& device, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage)
{
	auto props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	create(device, elemSz, nElems, usage, props);
}

void VulkanBuffer::initStaging(const VulkanDevice& device, void* data, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage)
{
	uint32_t totalSize = elemSz * nElems;

	VulkanBuffer stagingBuf;
	auto stagingBufType = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBuf.create(device, elemSz, nElems, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufType);
	stagingBuf.setData(data, 0, elemSz * nElems);

	auto bufUse = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
	auto bufType = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	create(device, elemSz, nElems, bufUse, bufType);

	stagingBuf.copyTo(*this);
}

uint32_t VulkanBuffer::findMemoryType(const VulkanDevice& device, VkMemoryPropertyFlags props, uint32_t typeFilter)
{
	auto devProps = device.getPhysicalDevice()->getMemProps();
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
