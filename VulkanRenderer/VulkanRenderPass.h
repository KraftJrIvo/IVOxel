#pragma once

#include "VulkanDevice.h"

class VulkanRenderPass
{
public:
	VulkanRenderPass();

	void init(const VulkanDevice& device, const VkFormat& depthStencilFormat, const VkFormat& surfaceFormat);
	
	void destroy(const VulkanDevice& device);

	VkRenderPass& get();

private:
	VkRenderPass _renderPass;
};