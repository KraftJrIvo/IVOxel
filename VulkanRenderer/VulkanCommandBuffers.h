#pragma once

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"

class VulkanCommandBuffers
{
public:
	void init(const VulkanDevice& device, int nSwapchainImgCount);
	void resize(std::size_t sz);
	VkCommandBuffer& operator[](std::size_t idx);
	VkCommandBuffer* data();

private:
	std::vector<VkCommandBuffer> _buffs;
};