#pragma once

#include "Window.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"

class VulkanSurface
{
public:
	void init(VulkanInstance& vulkan, Window& window);
	void initFormat(const VulkanDevice& device);
	void destroy(VulkanInstance& vulkan);

	VkSurfaceKHR& get();
	const VkSurfaceFormatKHR& getFormat() const;
	const VkSurfaceCapabilitiesKHR& getCapabs() const;

private:
	VkSurfaceKHR _surface;
	VkSurfaceFormatKHR _surfaceFormat;
	VkSurfaceCapabilitiesKHR _surfaceCapabs;
};