#include "VulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain()
{
}

void VulkanSwapchain::init(const VulkanDevice& device, VulkanSurface& surface, const std::pair<uint32_t, uint32_t>& winSz)
{
	uint32_t bufferSz = 2;
	auto& cabaps = surface.getCapabs();
	if (bufferSz < cabaps.minImageCount)
		bufferSz = cabaps.minImageCount;
	else if (cabaps.maxImageCount > 0 && bufferSz > cabaps.maxImageCount)
		bufferSz = cabaps.maxImageCount;

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t nPresentModes = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice()->get(), surface.get(), &nPresentModes, nullptr);
		std::vector<VkPresentModeKHR> presentModes(nPresentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice()->get(), surface.get(), &nPresentModes, presentModes.data());
		for (auto pm : presentModes)
			if (pm == VK_PRESENT_MODE_MAILBOX_KHR)
				present_mode = pm;
	}

	auto info = vkTypes::getSwapchainCreateInfo(surface.get(), surface.getFormat(), present_mode, bufferSz, winSz.first, winSz.second);

	vkCreateSwapchainKHR(device.get(), &info, nullptr, &_swapchain);
	vkGetSwapchainImagesKHR(device.get(), _swapchain, &_nImgs, nullptr);

	_swapchainImgs.resize(_nImgs);
	_swapchainImgViews.resize(_nImgs);

	vkGetSwapchainImagesKHR(device.get(), _swapchain, &_nImgs, _swapchainImgs.data());

	auto rgba = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentMapping mapping = { rgba, rgba, rgba, rgba };
	VkImageSubresourceRange subRng;
	subRng.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subRng.baseMipLevel = 0;
	subRng.levelCount = 1;
	subRng.baseArrayLayer = 0;
	subRng.layerCount = 1;

	for (uint32_t i = 0; i < _nImgs; ++i)
	{
		auto info = vkTypes::getImageViewCreateInfo(_swapchainImgs[i], mapping, subRng, surface.getFormat().format, VK_IMAGE_VIEW_TYPE_2D);
		vkCreateImageView(device.get(), &info, nullptr, &_swapchainImgViews[i]);
	}
}

void VulkanSwapchain::destroy(const VulkanDevice& device)
{
	for (uint32_t i = 0; i < _nImgs; ++i)
		vkDestroyImageView(device.get(), _swapchainImgViews[i], nullptr);
	vkDestroySwapchainKHR(device.get(), _swapchain, nullptr);
}

const VkSwapchainKHR& VulkanSwapchain::get()
{
	return _swapchain;
}

const std::vector<VkImageView>& VulkanSwapchain::getImgViews()
{
	return _swapchainImgViews;
}

int VulkanSwapchain::getImgCount()
{
	return _nImgs;
}
