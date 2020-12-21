#pragma once

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"

class VulkanFrameBuffers
{
public:
	void init(const VulkanDevice& device, VulkanRenderPass& renderPass, const std::pair<uint32_t, uint32_t>& winSz, const VkImageView& depthStencilImgView, const std::vector<VkImageView>& swapchainImgViews);
	void resize(std::size_t sz);
	VkFramebuffer& operator[](std::size_t idx);
	VkFramebuffer* data();

	void destroy(const VulkanDevice& device);

private:
	std::vector<VkFramebuffer> _buffs;
};