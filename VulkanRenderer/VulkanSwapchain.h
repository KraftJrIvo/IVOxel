#pragma once

#include "VulkanSurface.h"

class VulkanSwapchain
{
public:
	void init(const VulkanDevice& device, VulkanSurface& surface, const std::pair<uint32_t, uint32_t>& size, float renderScale);
	void destroy(const VulkanDevice& device);

	const VkSwapchainKHR& get();
	const std::vector<VkImage>& getImgs();
	const std::vector<VkImageView>& getImgViews();
	int getImgCount() const;

private:
	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _imgs;
	std::vector<VkImageView> _imgViews;
	unsigned int _nImgs;
};