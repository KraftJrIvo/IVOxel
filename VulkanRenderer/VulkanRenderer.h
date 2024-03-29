#pragma once

#include <AbstractRenderer.h>

#include <GameState.h>

#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanShaderManager.h"
#include "VulkanGeometryBuffers.h"
#include "VulkanDescriptorPool.h"
#include "VulkanSwapchain.h"
#include "VulkanImageBuffers.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanFrameBuffers.h"
#include "VulkanCommandBuffers.h"
#include "VulkanDepthStencilImage.h"

class VulkanRenderer : public AbstractRenderer
{
public:
	VulkanRenderer(Window& w, GameState& gs);
	~VulkanRenderer();
	
	void startRender() override;
	void stop() override;

private:
	VulkanInstance _vulkan;
	VulkanDevice _mainDevice;
	VulkanSurface _surface;
	VulkanSwapchain _swapchain;
	VulkanImageBuffers _images;
	VulkanPipeline _pipeline;
	VulkanDescriptorPool _descrPool;
	VulkanRenderPass _renderPass;
	VulkanDepthStencilImage _dsImg;
	VulkanShaderManager _shaderManager;

	VulkanGeometryBuffers _geomBuffs;
	VulkanFrameBuffers _frameBuffs;
	VulkanCommandBuffers _commandBuffs;

	uint32_t _currentSwapchainImgID;
	std::vector<VkSemaphore> _semImgAvailable;
	std::vector<VkSemaphore> _semRenderDone;
	std::vector<VkFence> _frameFences;
	std::vector<bool> _imgsInFlight;

	void _initSync();
	void _initShaders();

	void _clearEnv();
	void _initEnv();
	void _recreateEnv();

	VkFramebuffer& _getCurrentFrameBuffer();

	void _transitionImageLayout(VkCommandBuffer cb, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void _blitImage(VkCommandBuffer cb, VkImage image1, VkImage image2, uint32_t width, uint32_t height, float scale);

	void _beginRender(uint32_t frameID);
	void _endRender(uint32_t frameID);

	bool _runOnce();

	bool _firstInit = true;
};