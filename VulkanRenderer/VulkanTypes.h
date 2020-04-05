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

	VkWin32SurfaceCreateInfoKHR getSurfaceCreateInfo(const HINSTANCE& hinstance, const HWND& hwnd);

	VkSwapchainCreateInfoKHR getSwapchainCreateInfo(const VkSurfaceKHR& surface, const VkSurfaceFormatKHR& format, const VkPresentModeKHR& pm, uint32_t bufferSz, uint32_t width, uint32_t height);
	
	VkImageCreateInfo getImageCreateInfo(VkImageType type, VkFormat format, VkImageUsageFlagBits usage, uint32_t w, uint32_t h);
	
	VkImageViewCreateInfo getImageViewCreateInfo(const VkImage& img, const VkComponentMapping& mapping, const VkImageSubresourceRange& subRng, VkFormat format, VkImageViewType type);
	
	VkRenderPassCreateInfo getRenderPassCreateInfo(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subPasses);
	
	VkFramebufferCreateInfo getFramebufferCreateInfo(const VkRenderPass& renderPass, const std::vector<VkImageView>& attachments, uint32_t w, uint32_t h, uint32_t layers);
}