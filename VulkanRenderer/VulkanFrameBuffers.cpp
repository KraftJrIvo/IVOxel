#include "VulkanFrameBuffers.h"

void VulkanFrameBuffers::init(const VulkanDevice& device, VulkanRenderPass& renderPass, const std::pair<uint32_t, uint32_t>& size, 
	const VkImageView& depthStencilImgView, const std::vector<VkImageView>& swapchainImgViews)
{
	auto nSwapchainImgs = swapchainImgViews.size();
	_buffs.resize(nSwapchainImgs);

	std::vector<VkImageView> attachments(2);
	attachments[0] = depthStencilImgView;

	for (uint32_t i = 0; i < nSwapchainImgs; ++i)
	{
		attachments[1] = swapchainImgViews[i];
		auto info = vkTypes::getFramebufferCreateInfo(renderPass.get(), attachments, size.first, size.second, 1);

		vkCreateFramebuffer(device.get(), &info, nullptr, &_buffs[i]);
	}
}

void VulkanFrameBuffers::resize(std::size_t sz)
{
	_buffs.resize(sz);
}

VkFramebuffer& VulkanFrameBuffers::operator[](std::size_t idx)
{
	return _buffs[idx];
}

VkFramebuffer* VulkanFrameBuffers::data()
{
	return _buffs.data();
}

void VulkanFrameBuffers::destroy(const VulkanDevice& device)
{
	for (auto& buf : _buffs)
		vkDestroyFramebuffer(device.get(), buf, nullptr);
}
