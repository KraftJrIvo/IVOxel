#include "VulkanSurface.h"

VulkanSurface::VulkanSurface()
{
}

void VulkanSurface::init(VulkanInstance& vulkan, Window& window)
{
	auto info = vkTypes::getSurfaceCreateInfo(window.getHInstance(), window.getHWND());
	vkCreateWin32SurfaceKHR(vulkan.getInstance(), &info, nullptr, &_surface);
}

void VulkanSurface::destroy(VulkanInstance& vulkan)
{
	vkDestroySurfaceKHR(vulkan.getInstance(), _surface, nullptr);
}

void VulkanSurface::initFormat(const VulkanDevice& device)
{
	auto physDev = device.getPhysicalDevice()->get();
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDev, _surface, &_surfaceCapabs);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, _surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, _surface, &formatCount, formats.data());
	_surfaceFormat = formats[0];
}

VkSurfaceKHR& VulkanSurface::get()
{
	return _surface;
}

const VkSurfaceFormatKHR& VulkanSurface::getFormat() const
{
	return _surfaceFormat;
}

const VkSurfaceCapabilitiesKHR& VulkanSurface::getCapabs() const
{
	return _surfaceCapabs;
}
