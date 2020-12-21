#include "VulkanImageBuffers.h"

void VulkanImageBuffers::init(const VulkanDevice& device, const VulkanSurface& surface, const VulkanSwapchain& swapchain, const std::pair<uint32_t, uint32_t>& size, float renderScale)
{
	_nImgs = swapchain.getImgCount();

	_hostBufs.resize(_nImgs);
	for (auto& buf : _hostBufs)
		buf.initHost(device, 32 * size.first * size.second, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	auto imageInfo = vkTypes::getImageCreateInfo(VK_IMAGE_TYPE_2D, surface.getFormat().format, VkImageUsageFlagBits(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), size.first, size.second);
	
	_imgs.resize(_nImgs);
	_imgMems.resize(_nImgs);
	_imgViews.resize(_nImgs);

	auto rgba = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentMapping mapping = { rgba, rgba, rgba, rgba };
	VkImageSubresourceRange subRng;
	subRng.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subRng.baseMipLevel = 0;
	subRng.levelCount = 1;
	subRng.baseArrayLayer = 0;
	subRng.layerCount = 1;

	for (int i = 0; i < _nImgs; ++i)
	{
		vkCreateImage(device.get(), &imageInfo, nullptr, &_imgs[i]);
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.get(), _imgs[i], &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanBuffer::findMemoryType(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memRequirements.memoryTypeBits);
		vkAllocateMemory(device.get(), &allocInfo, nullptr, &_imgMems[i]);
		vkBindImageMemory(device.get(), _imgs[i], _imgMems[i], 0);
	
		auto info = vkTypes::getImageViewCreateInfo(_imgs[i], mapping, subRng, surface.getFormat().format, VK_IMAGE_VIEW_TYPE_2D);
		vkCreateImageView(device.get(), &info, nullptr, &_imgViews[i]);
	}
}

void VulkanImageBuffers::destroy(const VulkanDevice& device)
{
	for (uint32_t i = 0; i < _nImgs; ++i)
	{
		vkDestroyImageView(device.get(), _imgViews[i], nullptr);
		vkDestroyImage(device.get(), _imgs[i], nullptr);
		vkFreeMemory(device.get(), _imgMems[i], nullptr);
	}
	_hostBufs.clear();
}

const std::vector<VkImageView>& VulkanImageBuffers::getImgViews()
{
	return _imgViews;
}

VkImage& VulkanImageBuffers::operator[](std::size_t idx)
{
	return _imgs[idx];
}

VkBuffer VulkanImageBuffers::getBuf(uint32_t idx)
{
	return _hostBufs[idx].getBuffer();
}
