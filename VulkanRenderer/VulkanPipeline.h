#pragma once

#include "VulkanRenderPass.h"

class VulkanPipeline
{
public:
	void init(const VulkanDevice& device, VulkanRenderPass& renderPass, const VkRect2D& rect,
		const std::vector<VkDescriptorSetLayout>& dsls, const std::vector<VkPipelineShaderStageCreateInfo>& shaderInfos);
	void destroy(const VulkanDevice& device);

	const VkPipeline& get();
	VkPipelineLayout& getLayout();

private:
	VkPipelineLayout _layout;
	VkPipeline _pipeline;
};