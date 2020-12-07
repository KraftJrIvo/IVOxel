#pragma once

#include "VulkanSurface.h"

class VulkanSwapchain
{
public:
	VulkanSwapchain();

	void init(const VulkanDevice& device, VulkanSurface& surface, const std::pair<uint32_t, uint32_t>& winSz);

	void destroy(const VulkanDevice& device);

	const VkSwapchainKHR& get();
	const std::vector<VkImageView>& getImgViews();
	int getImgCount();

private:
	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _swapchainImgs;
	std::vector<VkImageView> _swapchainImgViews;
	unsigned int _nImgs;
};