#include "VulkanPipeline.h"
#include "Vertex.h"

VulkanPipeline::VulkanPipeline()
{

}

void VulkanPipeline::init(const VulkanDevice& device, VulkanRenderPass& renderPass, const VkRect2D& rect,
	const std::vector<VkDescriptorSetLayout>& dsls, const std::vector<VkPipelineShaderStageCreateInfo>& shaderInfos)
{
	auto vertexBindingsDescr = Vertex::getBindingDescriptions();
	auto vertexAttribDescr = Vertex::getAttributeDescriptions();
	auto vertexISCreateInfo = vkTypes::getPipelineVertexISCreateInfo(vertexBindingsDescr, vertexAttribDescr);

	auto inputAssemblyCreateInfo = vkTypes::getPipelineInputAssemblyISCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = rect.extent.width;
	viewport.height = rect.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	std::vector<VkViewport> viewports = { viewport };
	std::vector<VkRect2D> scissors = { rect };
	auto viewportInfo = vkTypes::getPipelineViewportSCreateInfo(viewports, scissors);
	auto rasterizationInfo = vkTypes::getPipelineRasterizationSCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, 1.0f, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	auto multisampleInfo = vkTypes::getPipelineMultisampleSCreateInfo();

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// for alpha
	//colorBlendAttachment.blendEnable		 =	VK_TRUE;
	//colorBlendAttachment.srcColorBlendFactor =	VK_BLEND_FACTOR_SRC_ALPHA;
	//colorBlendAttachment.dstColorBlendFactor =	VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//colorBlendAttachment.colorBlendOp		 =	VK_BLEND_OP_ADD;
	//colorBlendAttachment.srcAlphaBlendFactor =	VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment.dstAlphaBlendFactor =	VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment.alphaBlendOp		 =	VK_BLEND_OP_ADD;

	std::vector<VkPipelineColorBlendAttachmentState> attachments = { colorBlendAttachment };

	auto colorBlendInfo = vkTypes::getPipelineColorBlendSCreateInfo(attachments);
	auto depthStencilInfo = vkTypes::getPipelineDepthStencilSCreateInfo();

	auto layoutInfo = vkTypes::getPipelineLayoutCreateInfo(dsls, {});
	vkCreatePipelineLayout(device.get(), &layoutInfo, nullptr, &_layout);

	auto pipelineInfo = vkTypes::getGraphicsPipelineCreateInfo(_layout, renderPass.get(), 0, shaderInfos,
		vertexISCreateInfo, inputAssemblyCreateInfo, viewportInfo, rasterizationInfo, multisampleInfo, colorBlendInfo, &depthStencilInfo);
	vkCreateGraphicsPipelines(device.get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline);
}

void VulkanPipeline::destroy(const VulkanDevice& device)
{
	vkDestroyPipeline(device.get(), _pipeline, nullptr);
	vkDestroyPipelineLayout(device.get(), _layout, nullptr);
}

const VkPipeline& VulkanPipeline::get()
{
	return _pipeline;
}

VkPipelineLayout& VulkanPipeline::getLayout()
{
	return _layout;
}
