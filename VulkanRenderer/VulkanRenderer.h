#pragma once

#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanFrameBuffers.h"
#include "VulkanCommandBuffers.h"
#include "VulkanDepthStencilImage.h"

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
	VulkanSurface _surface;
	VulkanSwapchain _swapchain;
	VulkanPipeline _pipeline;
	VulkanRenderPass _renderPass;
	VulkanDepthStencilImage _dsImg;

	VulkanFrameBuffers _frameBuffs;
	VulkanCommandBuffers _commandBuffs;
	VulkanBuffer _vertexBuff;
	VulkanBuffer _indexBuff;
	std::vector<VulkanBuffer> _uniformBuffs;

	uint32_t _currentSwapchainImgID;

	std::vector<VkSemaphore> _semImgAvailable;
	std::vector<VkSemaphore> _semRenderDone;
	std::vector<VkFence> _frameFences;
	std::vector<bool> _imgsInFlight;

	VulkanShader _vertexShader;
	VulkanShader _fragmentShader;

	VkDescriptorSetLayout _viewShaderInfoDescriptorSetLayout;
	VkDescriptorSetLayout _lightingShaderInfoDescriptorSetLayout;
	VkDescriptorSetLayout _mapShaderInfoDescriptorSetLayout;
	std::vector<VkDescriptorSet> _viewShaderInfoDescriptorSets;
	std::vector<VkDescriptorSet> _lightingShaderInfoDescriptorSets;
	std::vector<VkDescriptorSet> _mapShaderInfoDescriptorSets;
	VkDescriptorPool _descriptorPool;

	ViewShaderInfo _viewShaderInfo;
	LightingShaderInfo _lightingShaderInfo;
	MapShaderInfo _mapShaderInfo;

	glm::vec2 _curRot;
	glm::vec3 _curTrans;

	void _initDescriptorSetLayout(VkDescriptorSetLayout& layout);
	void _initDescriptorPool();
	void _initDescriptorSet(VkDescriptorSetLayout& layout, std::vector<VkDescriptorSet>& set, uint32_t range, uint32_t offset);
	void _initStageBuffer(void* data, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage, VulkanBuffer& buf);
	void _initHostBuffer(uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage, VulkanBuffer& buf);
	void _initSync();
	void _initShaders();

	void _clearEnv();
	void _initEnv();
	void _recreateEnv();

	void _updateViewShaderInfo(uint32_t idx);
	void _updateMapShaderInfo(const VoxelMap& map, uint32_t idx);
	void _updateLightingShaderInfo(const VoxelMap& map, uint32_t idx);

	VkFramebuffer& _getCurrentFrameBuffer();

	bool _keepGoing = true;
};