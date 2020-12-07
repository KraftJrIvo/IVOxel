#include "VulkanCommandBuffers.h"

VulkanCommandBuffers::VulkanCommandBuffers()
{
}

void VulkanCommandBuffers::init(const VulkanDevice& device, int nSwapchainImgCount)
{
	_buffs.resize(nSwapchainImgCount);
	device.getCommand(_buffs.data(), nSwapchainImgCount, device.getQFIdByType(VK_QUEUE_GRAPHICS_BIT));
}

void VulkanCommandBuffers::resize(std::size_t sz)
{
	_buffs.resize(sz);
}

VkCommandBuffer& VulkanCommandBuffers::operator[](std::size_t idx)
{
	return _buffs[idx];
}

VkCommandBuffer* VulkanCommandBuffers::data()
{
	return _buffs.data();
}
