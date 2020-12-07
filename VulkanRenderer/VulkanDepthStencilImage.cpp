#include "VulkanDepthStencilImage.h"

VulkanDepthStencilImage::VulkanDepthStencilImage()
{
}

void VulkanDepthStencilImage::init(const VulkanDevice& device, VulkanSurface& surface)
{
	std::vector<VkFormat> formatsToTry =
	{
		//VK_FORMAT_D32_SFLOAT,
		//VK_FORMAT_D16_UNORM,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_X8_D24_UNORM_PACK32,
		VK_FORMAT_S8_UINT
	};

	_format = VK_FORMAT_UNDEFINED;
	VkFormatProperties fProps;
	for (auto& format : formatsToTry)
	{
		vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice()->get(), format, &fProps);
		if (fProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			_format = format;
			break;
		}
	}

	auto& capabs = surface.getCapabs();
	auto imgInfo = vkTypes::getImageCreateInfo(VK_IMAGE_TYPE_2D, _format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		capabs.currentExtent.width, capabs.currentExtent.height);

	vkCreateImage(device.get(), &imgInfo, nullptr, &_img);

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(device.get(), _img, &memReq);

	uint32_t memId = _getMemoryId(device, memReq, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	auto allocInfo = vkTypes::getMemAllocInfo(memReq.size, memId);
	vkAllocateMemory(device.get(), &allocInfo, nullptr, &_imgMem);
	vkBindImageMemory(device.get(), _img, _imgMem, 0);

	auto rgba = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentMapping mapping = { rgba, rgba, rgba, rgba };
	VkImageSubresourceRange subRng;
	subRng.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	subRng.baseMipLevel = 0;
	subRng.levelCount = 1;
	subRng.baseArrayLayer = 0;
	subRng.layerCount = 1;

	auto viewInfo = vkTypes::getImageViewCreateInfo(_img, mapping, subRng, _format, VK_IMAGE_VIEW_TYPE_2D);

	vkCreateImageView(device.get(), &viewInfo, nullptr, &_imgView);
}

void VulkanDepthStencilImage::destroy(const VulkanDevice& device)
{
	vkDestroyImageView(device.get(), _imgView, nullptr);
	vkFreeMemory(device.get(), _imgMem, nullptr);
	vkDestroyImage(device.get(), _img, nullptr);
}

const VkImage& VulkanDepthStencilImage::get()
{
	return _img;
}

const VkFormat& VulkanDepthStencilImage::getFormat()
{
	return _format;
}

const VkImageView& VulkanDepthStencilImage::getImgView()
{
	return _imgView;
}

uint32_t VulkanDepthStencilImage::_getMemoryId(const VulkanDevice& device, const VkMemoryRequirements& memReq, VkMemoryPropertyFlags reqFlags)
{
	auto devMemProps = device.getPhysicalDevice()->getMemProps();
	for (uint32_t i = 0; i < devMemProps.memoryTypeCount; ++i)
		if (memReq.memoryTypeBits & (1 << i))
			if ((devMemProps.memoryTypes[i].propertyFlags & reqFlags) == reqFlags)
				return i;
	return -1;
}
