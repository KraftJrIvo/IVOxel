#pragma once

#include "VulkanInstance.h"
#include "VulkanShader.h"
#include "VulkanBuffer.h"

#include "ViewShaderInfo.h"
#include "MapShaderInfo.h"
#include "LightingShaderInfo.h"

#include <VoxelMap.h>

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
	std::vector<VulkanBuffer> _uniformBuffs;

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
	VkDescriptorSetLayout _viewShaderInfoDescriptorSetLayout;
	VkDescriptorSetLayout _lightingShaderInfoDescriptorSetLayout;
	VkDescriptorSetLayout _mapShaderInfoDescriptorSetLayout;
	std::vector<VkDescriptorSet> _viewShaderInfoDescriptorSets;
	std::vector<VkDescriptorSet> _lightingShaderInfoDescriptorSets;
	std::vector<VkDescriptorSet> _mapShaderInfoDescriptorSets;
	VkDescriptorPool _descriptorPool;
	VkPipelineLayout _pipelineLayout;

	ViewShaderInfo _viewShaderInfo;
	LightingShaderInfo _lightingShaderInfo;
	MapShaderInfo _mapShaderInfo;

	glm::vec2 _curRot;

	void _initSurface();
	void _initSwapchain();
	void _initSwapchainImages();
	void _initDepthStencilImage();
	void _initRenderPass();
	void _initFrameBuffers();
	void _initCommandBuffers();
	void _initDescriptorSetLayout(VkDescriptorSetLayout& layout);
	void _initDescriptorPool();
	void _initDescriptorSet(VkDescriptorSetLayout& layout, std::vector<VkDescriptorSet>& set, uint32_t range, uint32_t offset);
	void _initStageBuffer(void* data, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage, VulkanBuffer& buf);
	void _initHostBuffer(uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage, VulkanBuffer& buf);
	void _initSync();
	void _initPipeline();
	void _initShaders();
	void _setSurfaceFormat();

	void _clearEnv();
	void _initEnv();
	void _recreateEnv();

	void _updateViewShaderInfo(uint32_t idx);
	void _updateMapShaderInfo(const VoxelMap& map, uint32_t idx);
	void _updateLightingShaderInfo(const VoxelMap& map, uint32_t idx);

	VkFramebuffer& _getCurrentFrameBuffer();

	uint32_t _getMemoryId(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags reqFlags);

	bool _keepGoing = true;
};