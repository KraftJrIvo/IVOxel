#include "VulkanRenderPass.h"

VulkanRenderPass::VulkanRenderPass()
{
}

void VulkanRenderPass::init(const VulkanDevice& device, const VkFormat& depthStencilFormat, const VkFormat& surfaceFormat)
{
	std::vector<VkAttachmentDescription> attachments(2);
	attachments[0].flags = 0;
	attachments[0].format = depthStencilFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachments[1].flags = 0;
	attachments[1].format = surfaceFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkAttachmentReference> colorAttachments(1);
	colorAttachments[0].attachment = 1;
	colorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription> subpasses(1);
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = colorAttachments.size();
	subpasses[0].pColorAttachments = colorAttachments.data();		// layout(location=0) out vec4 FinalColor;
	subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

	auto info = vkTypes::getRPCreateInfo(attachments, subpasses);

	vkCreateRenderPass(device.get(), &info, nullptr, &_renderPass);
}

void VulkanRenderPass::destroy(const VulkanDevice& device)
{
	vkDestroyRenderPass(device.get(), _renderPass, nullptr);
}

VkRenderPass& VulkanRenderPass::get()
{
	return _renderPass;
}
