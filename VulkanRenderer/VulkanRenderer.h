#pragma once

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
	VulkanImageBuffers _images;
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

	VkCommandBuffer _beginSingleTimeCommands();
	void _endSingleTimeCommands(const VkCommandBuffer& commandBuffer);
	void _transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void _copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void _copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height);
	void _scaledImageToImage(VkImage image1, VkImage image2, uint32_t width, uint32_t height, float scale);

	bool _keepGoing = true;
	bool _firstInit = true;
};