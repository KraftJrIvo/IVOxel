#pragma once

#include "VulkanSurface.h"

class VulkanDepthStencilImage
{
public:
	void init(const VulkanDevice& device, VulkanSurface& surface);
	void destroy(const VulkanDevice& device);

	const VkImage& get();
	const VkFormat& getFormat();
	const VkImageView& getImgView();

private:
	VkImage _img;
	VkDeviceMemory _imgMem;
	VkImageView _imgView;
	VkFormat _format;

	uint32_t _getMemoryId(const VulkanDevice& device, const VkMemoryRequirements& memReq, VkMemoryPropertyFlags reqFlags);
};