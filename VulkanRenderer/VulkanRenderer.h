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
	VkSurfaceCapabilitiesKHR _surfaceCapabs;
	VkSwapchainKHR _swapchain;

	std::vector<VkImage> _swapchainImgs;
	std::vector<VkImageView> _swapchainImgViews;

	uint32_t _swapchainImgCount;

	void _initSurface();
	void _initSwapchain();
	void _initSwapchainImages();
	void _setSurfaceFormat(const VulkanDevice&);

	bool _keepGoing = true;
};