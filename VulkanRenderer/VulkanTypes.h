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
	VkCommandBufferBeginInfo getCBBeginInfo(VkCommandBufferUsageFlags flags);
	VkBufferCreateInfo getBufCreateInfo(VkDeviceSize size, VkBufferUsageFlags useFlags);
	VkMemoryAllocateInfo getMemAllocInfo(VkDeviceSize size, uint32_t memTypeId);
	VkWin32SurfaceCreateInfoKHR getSurfaceCreateInfo(const HINSTANCE& hinstance, const HWND& hwnd);
	VkSwapchainCreateInfoKHR getSwapchainCreateInfo(const VkSurfaceKHR& surface, const VkSurfaceFormatKHR& format, const VkPresentModeKHR& pm, uint32_t bufferSz, uint32_t width, uint32_t height);
	VkImageCreateInfo getImageCreateInfo(VkImageType type, VkFormat format, VkImageUsageFlagBits usage, uint32_t w, uint32_t h);
	VkImageViewCreateInfo getImageViewCreateInfo(const VkImage& img, const VkComponentMapping& mapping, const VkImageSubresourceRange& subRng, VkFormat format, VkImageViewType type);
	VkRenderPassCreateInfo getRPCreateInfo(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subPasses);
	VkRenderPassBeginInfo getRPBeginInfo(const VkRenderPass& renderPass, const VkFramebuffer& frameBuff, const VkRect2D& area, const std::vector<VkClearValue>& clearVals);
	VkFramebufferCreateInfo getFramebufferCreateInfo(const VkRenderPass& renderPass, const std::vector<VkImageView>& attachments, uint32_t w, uint32_t h, uint32_t layers);
	VkFenceCreateInfo getFenceCreateInfo();
	VkPresentInfoKHR getPresentInfo(const std::vector<VkSemaphore>& semaphores, const std::vector<VkSwapchainKHR>& swapchains, const std::vector<uint32_t>& imageIds, std::vector<VkResult>& results);
	VkSubmitInfo getSubmitInfo(const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& signalSemaphores, const std::vector<VkCommandBuffer>& cmdBuffs, uint32_t* dstStageMaskFlags);
	VkSemaphoreCreateInfo getSemaphoreCreateInfo();
	VkDescriptorSetLayoutCreateInfo getDSLCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	VkDescriptorPoolCreateInfo getDescriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& descPoolSizes, uint32_t maxSets);
	VkDescriptorSetAllocateInfo getDescriptorSetAllocateInfo(const VkDescriptorPool& pool, const std::vector<VkDescriptorSetLayout>& layouts, uint32_t setCount);
	VkPipelineVertexInputStateCreateInfo getPipelineVertexISCreateInfo(const std::vector<VkVertexInputBindingDescription>& vibDescrs, const std::vector<VkVertexInputAttributeDescription>& vaDescrs);
	VkPipelineInputAssemblyStateCreateInfo getPipelineInputAssemblyISCreateInfo(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnabled = VK_FALSE);
	VkPipelineViewportStateCreateInfo getPipelineViewportSCreateInfo(const std::vector<VkViewport>& viewports, const std::vector<VkRect2D>& scissors);
	VkPipelineRasterizationStateCreateInfo getPipelineRasterizationSCreateInfo(VkPolygonMode polyMode = VK_POLYGON_MODE_FILL, VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT, uint32_t lineWidth = 1.0f, VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE);
	VkPipelineMultisampleStateCreateInfo getPipelineMultisampleSCreateInfo(VkBool32 shading = VK_FALSE, VkSampleCountFlagBits sampleFlags = VK_SAMPLE_COUNT_1_BIT, float minSampleShading = 1.0f, const VkSampleMask* pMask = nullptr, VkBool32 alphaToCoverageEnable = VK_FALSE, VkBool32 alphaToOneEnable = VK_FALSE);
	VkPipelineColorBlendStateCreateInfo getPipelineColorBlendSCreateInfo(const std::vector<VkPipelineColorBlendAttachmentState>& attachments, VkBool32 logicOpEnable = VK_FALSE, VkLogicOp logicOp = VK_LOGIC_OP_COPY, std::vector<float> blendConsts = {0,0,0,0});
	VkPipelineDepthStencilStateCreateInfo getPipelineDepthStencilSCreateInfo(VkBool32 depthTest = VK_TRUE, VkBool32 depthWrite = VK_TRUE, VkCompareOp compareOp = VK_COMPARE_OP_LESS, VkBool32 depthBoundsTest = VK_FALSE, VkBool32 stencilTest = VK_FALSE, VkStencilOpState front = {}, VkStencilOpState back = {}, float minBound = 0.0f, float maxBound = 1.0f);
	VkPipelineLayoutCreateInfo getPipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout>& layouts, const std::vector<VkPushConstantRange>& constantRanges);
	VkGraphicsPipelineCreateInfo getGraphicsPipelineCreateInfo(const VkPipelineLayout& layout, const VkRenderPass& renderPass, uint32_t subpass, const std::vector<VkPipelineShaderStageCreateInfo>& stages, const VkPipelineVertexInputStateCreateInfo& vertexInputSCI, const VkPipelineInputAssemblyStateCreateInfo& inputAssemblySCI, const VkPipelineViewportStateCreateInfo& viewportSCI, const VkPipelineRasterizationStateCreateInfo& rasterizationSCI, const VkPipelineMultisampleStateCreateInfo& multisampleSCI, const VkPipelineColorBlendStateCreateInfo& colorBlendSCI, const VkPipelineDepthStencilStateCreateInfo* depthStencilSCI = nullptr, const VkPipelineDynamicStateCreateInfo* dynamicSCI = nullptr, VkPipeline basePipeline = VK_NULL_HANDLE, uint32_t basePipelineId = -1);
}