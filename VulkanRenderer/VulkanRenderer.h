#pragma once

#include "VulkanInstance.h"
#include "VulkanShader.h"

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

	VkImage _depthStencilImg;
	VkDeviceMemory _depthStencilImgMem;
	VkImageView _depthStencilImgView;

	uint32_t _swapchainImgCount;
	uint32_t _currentSwapchainImgID;

	std::vector<VkSemaphore> _semImgAvailable;
	std::vector<VkSemaphore> _semRenderDone;
	std::vector<VkFence> _frameFences;
	std::vector<bool> _imgsInFlight;

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
	void _initSync();
	void _initPipeline();
	void _initShaders();
	void _setSurfaceFormat();
	
	VkFramebuffer& _getCurrentFrameBuffer();

	uint32_t _getMemoryId(const VkMemoryRequirements& memReq, VkMemoryPropertyFlagBits reqFlags);

	bool _keepGoing = true;
};