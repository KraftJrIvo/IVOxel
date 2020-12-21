#pragma once

#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanShaderManager.h"
#include "VulkanGeometryBuffers.h"
#include "VulkanDescriptorPool.h"
#include "VulkanSwapchain.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanFrameBuffers.h"
#include "VulkanCommandBuffers.h"
#include "VulkanDepthStencilImage.h"
#include "GameState.h"

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();
	
	void init();
	void run(const VoxelMap& map);
	void stop();
	bool runOnce();

	void beginRender(uint32_t frameID);
	void endRender(uint32_t frameID);

private:
	VulkanInstance _vulkan;
	VulkanDevice _mainDevice;
	VulkanSurface _surface;
	VulkanSwapchain _swapchain;
	VulkanPipeline _pipeline;
	VulkanDescriptorPool _descrPool;
	VulkanRenderPass _renderPass;
	VulkanDepthStencilImage _dsImg;
	VulkanShaderManager _shaderManager;

	VulkanGeometryBuffers _geomBuffs;
	VulkanFrameBuffers _frameBuffs;
	VulkanCommandBuffers _commandBuffs;

	GameState _game;

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

	bool _keepGoing = true;
	bool _firstInit = true;
};