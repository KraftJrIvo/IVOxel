#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace vkTypes
{
	VkApplicationInfo getAppInfo();

	VkInstanceCreateInfo getInstanceCreateInfo(const VkApplicationInfo& appInfo, const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

	VkDeviceQueueCreateInfo getQFCreateInfo(uint32_t familyId, float* priority);

	VkDeviceCreateInfo getDeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueInfos, const VkPhysicalDeviceFeatures* feats, 
		const std::vector<const char*>& layers, const std::vector<const char*>& extensions);
}