#pragma once

#include <vector>
#include <windows.h>
#include <vulkan/vulkan.h>

namespace vkTypes
{
	VkApplicationInfo getAppInfo();

	VkInstanceCreateInfo getInstanceCreateInfo(const VkApplicationInfo& appInfo, const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

	VkDeviceQueueCreateInfo getQFCreateInfo(uint32_t queueCount, uint32_t familyId, float* priority);

	VkDeviceCreateInfo getDeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueInfos, const VkPhysicalDeviceFeatures* feats, 
		const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

	VkCommandPoolCreateInfo getCPCreateInfo(uint32_t familyId, VkCommandPoolCreateFlags flags = 0);
	
	VkCommandBufferAllocateInfo getCBAllocateInfo(const VkCommandPool& pool, uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkBufferCreateInfo getBufCreateInfo(VkDeviceSize size, VkBufferUsageFlags useFlags);
	
	VkMemoryAllocateInfo getMemAllocInfo(VkDeviceSize size, uint32_t memTypeId);

	VkWin32SurfaceCreateInfoKHR getWin32SurfaceCreateInfo(const HINSTANCE& hinstance, const HWND& hwnd);
}