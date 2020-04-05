#pragma once

#include "VulkanInstance.h"

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();
	
	void init();
	void run();
	void stop();
	bool runOnce();


private:
	VulkanInstance _vulkan;
	VkSurfaceKHR _surface;
	VkSurfaceFormatKHR _surfaceFormat;
	VkFormat _depthStencilFormat;
	VkSurfaceCapabilitiesKHR _surfaceCapabs;
	VkSwapchainKHR _swapchain;
	VkRenderPass _renderPass;

	std::vector<VkImage> _swapchainImgs;
	std::vector<VkImageView> _swapchainImgViews;
	std::vector<VkFramebuffer> _frameBuffs;

	VkImage _depthStencilImg;
	VkDeviceMemory _depthStencilImgMem;
	VkImageView _depthStencilImgView;

	uint32_t _swapchainImgCount;

	void _initSurface();
	void _initSwapchain();
	void _initSwapchainImages();
	void _initDepthStencilImage(const VulkanDevice&);
	void _initRenderPass(const VulkanDevice&);
	void _initFrameBuffers(const VulkanDevice&);
	void _setSurfaceFormat(const VulkanDevice&);

	uint32_t _getMemoryId(const VulkanDevice& device, const VkMemoryRequirements& memReq, VkMemoryPropertyFlagBits reqFlags);

	bool _keepGoing = true;
};