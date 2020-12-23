#pragma once

#include "VulkanBuffer.h"
#include "VulkanSwapchain.h"

class VulkanImageBuffers
{
public:
	void init(const VulkanDevice& device, const VulkanSurface& surface, const VulkanSwapchain& swapchain, const std::pair<uint32_t, uint32_t>& size, float renderScale);
	void destroy(const VulkanDevice& device);

	const std::vector<VkImageView>& getImgViews();
	VkImage& operator[](std::size_t idx);

private:
	std::vector<VkImage> _imgs;
	std::vector<VkDeviceMemory> _imgMems;
	std::vector<VkImageView> _imgViews;
	unsigned int _nImgs;
};