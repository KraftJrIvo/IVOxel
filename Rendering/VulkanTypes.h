#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace vkTypes
{
	VkApplicationInfo getAppInfo();

	VkInstanceCreateInfo getInstanceCreateInfo(const VkApplicationInfo& appInfo, std::vector<const char*>& layers, std::vector<const char*>& extensions);
}