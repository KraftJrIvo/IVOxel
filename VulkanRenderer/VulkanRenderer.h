#pragma once

#include "VulkanInstance.h"
#include "VulkanShader.h"
#include "VulkanBuffer.h"

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();
	
	void init();
	void run();
	void stop();
	bool runOnce();

	void beginRender(uint32_t frameID);
	void endRender(uint32_t frameID);

private:
	VulkanInstance _vulkan;
	VulkanDevice _mainDevice;
	VkSurfaceKHR _surface;
	VkSurfaceFormatKHR _surfaceFormat;
	VkFormat _depthStencilFormat;
	VkSurfaceCapabilitiesKHR _surfaceCapabs;
	VkSwapchainKHR _swapchain;
	VkRenderPass _renderPass;

	std::vector<VkImage> _swapchainImgs;
	std::vector<VkImageView> _swapchainImgViews;
	std::vector<VkFramebuffer> _frameBuffs;
	VulkanBuffer _vertexBuff;
	VulkanBuffer _indexBuff;

	VkImage _depthStencilImg;
	VkDeviceMemory _depthStencilImgMem;
	VkImageView _depthStencilImgView;

	uint32_t _swapchainImgCount;
	uint32_t _currentSwapchainImgID;

	std::vector<VkSemaphore> _semImgAvailable;
	std::vector<VkSemaphore> _semRenderDone;
	std::vector<VkFence> _frameFences;
	std::vector<bool> _imgsInFlight;

	std::vector<VkCommandBuffer> _commandBufs;

	VulkanShader _vertexShader;
	VulkanShader _fragmentShader;

	VkPipeline _pipeline;
	VkPipelineLayout _pipelineLayout;

	void _initSurface();
	void _initSwapchain();
	void _initSwapchainImages();
	void _initDepthStencilImage();
	void _initRenderPass();
	void _initFrameBuffers();
	void _initCommandBuffers();
	void _initStageBuffer(void* data, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage, VulkanBuffer& buf);
	void _initSync();
	void _initPipeline();
	void _initShaders();
	void _setSurfaceFormat();

	void _clearEnv();
	void _initEnv();
	void _recreateEnv();

	VkFramebuffer& _getCurrentFrameBuffer();

	uint32_t _getMemoryId(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags reqFlags);

	bool _keepGoing = true;
};