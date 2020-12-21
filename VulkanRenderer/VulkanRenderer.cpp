#include "VulkanRenderer.h"
#include "Window.h"
#include "Vertex.h"

#include "FPSCounter.h"
#include "FPSLimiter.h"

#include <iostream>
#include <thread>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

Window window(512, 512, L"test");

#define FPS_LIMIT 144

VulkanRenderer::VulkanRenderer() :
#ifdef _DEBUG
	_vulkan(
		{ "VK_LAYER_KHRONOS_validation" }, 
		{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, "VK_EXT_debug_report" }
	)
#else
	_vulkan(
		{},
		{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME }
	)
#endif
{
	_surface.init(_vulkan, window);
}

VulkanRenderer::~VulkanRenderer()
{
	auto device = _vulkan.getDevice().get();

	for (uint32_t i = 0; i < _swapchain.getImgCount(); ++i)
	{
		vkDestroySemaphore(_mainDevice.get(), _semImgAvailable[i], nullptr);
		vkDestroySemaphore(_mainDevice.get(), _semRenderDone[i], nullptr);
		vkDestroyFence(_mainDevice.get(), _frameFences[i], nullptr);
	}

	_shaderManager.destroy(_mainDevice);
	_descrPool.destroy(_mainDevice);

	_clearEnv();
}

void VulkanRenderer::init()
{
	std::vector<uint32_t> queueFamilies = { VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT };
	std::vector<VkPhysicalDeviceType> deviceTypesByPriority = { VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU };

	_vulkan.chooseDevice(queueFamilies, deviceTypesByPriority, _surface.get());

	_mainDevice = _vulkan.getDevice();

	if (_mainDevice.get() == VK_NULL_HANDLE)
		std::cout << std::endl << "No device was chosen." << std::endl;
	else
	{
		std::cout << std::endl << "Device was chosen: " << _vulkan.getPhysDevice().getProps().deviceName << std::endl;
		std::cout << std::endl << "Initializing Vulkan environment..." << std::endl;

		_initEnv();

		std::cout << std::endl << "Initialized!" << std::endl;

		_currentSwapchainImgID = -1;
	}
}

void VulkanRenderer::run(const VoxelMap& map)
{
	_game.setMap(map);

	auto queue = _mainDevice.getQueueByType(VK_QUEUE_GRAPHICS_BIT);

	auto cbBeginInfo = vkTypes::getCBBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkRect2D renderArea = window.getRenderArea();
	std::vector<VkClearValue> clearVals(2);
	clearVals[0].depthStencil.depth = 1.0f;
	clearVals[0].depthStencil.stencil = 0;
	clearVals[1].color.float32[0] = 0.0f;
	clearVals[1].color.float32[1] = 0.0f;
	clearVals[1].color.float32[2] = 0.0f;
	clearVals[1].color.float32[3] = 0.0f;

	VkRenderPassBeginInfo rpBeginInfo;

	FPSCounter fpsCounter;
	FPSLimiter fpsLimiter(FPS_LIMIT);
	fpsLimiter.reset();

	uint32_t curFrameID = 0;

	while (runOnce())
	{
		fpsCounter.tellFPS(1000);

		if (window.wasResized())
		{
			do
			{
				renderArea = window.getRenderArea();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				window.Update();
			} 
			while (!renderArea.extent.width || !renderArea.extent.height);
			_recreateEnv();
		}

		beginRender(curFrameID);

		if (_imgsInFlight[curFrameID]) {
			vkWaitForFences(_mainDevice.get(), 1, &_frameFences[curFrameID], VK_TRUE, UINT64_MAX);
		}

		_imgsInFlight[curFrameID] = true;
		
		vkBeginCommandBuffer(_commandBuffs[curFrameID], &cbBeginInfo);

		rpBeginInfo = vkTypes::getRPBeginInfo(_renderPass.get(), _getCurrentFrameBuffer(), renderArea, clearVals);
		vkCmdBeginRenderPass(_commandBuffs[curFrameID], &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_commandBuffs[curFrameID], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.get());

		_geomBuffs.bindCmdBuffer(_mainDevice, _commandBuffs[curFrameID]);

		auto& sets = _descrPool.getSets(curFrameID);
		vkCmdBindDescriptorSets(_commandBuffs[curFrameID], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.getLayout(), 0, 3, sets.data(), 0, nullptr);
		vkCmdDrawIndexed(_commandBuffs[curFrameID], _geomBuffs.getIndicesCount(), 1, 0, 0, 0);

		vkCmdEndRenderPass(_commandBuffs[curFrameID]);

		vkEndCommandBuffer(_commandBuffs[curFrameID]);

		vkResetFences(_mainDevice.get(), 1, &_frameFences[curFrameID]);

		_game.update(_descrPool, curFrameID);

		std::vector<VkSemaphore> waitSemaphores = {_semImgAvailable[curFrameID]};
		std::vector<VkSemaphore> signalSemaphores = {_semRenderDone[curFrameID]};
		std::vector<VkCommandBuffer> cmdBufs = { _commandBuffs[curFrameID] };
		VkSubmitInfo submitInfo = vkTypes::getSubmitInfo(waitSemaphores, signalSemaphores, cmdBufs, waitStages);
		vkQueueSubmit(queue, 1, &submitInfo, _frameFences[curFrameID]);

		endRender(curFrameID);

		curFrameID = (curFrameID + 1) % _swapchain.getImgCount();

		if (FPS_LIMIT)
			fpsLimiter.tick();
	}

	vkQueueWaitIdle(queue);
}

bool VulkanRenderer::runOnce()
{
	return window.Update();
}

void VulkanRenderer::beginRender(uint32_t frameID)
{
	auto& dev = _mainDevice.get();
	vkWaitForFences(dev, 1, &_frameFences[frameID], VK_TRUE, UINT64_MAX);
	vkAcquireNextImageKHR(dev, _swapchain.get(), UINT64_MAX, _semImgAvailable[frameID], VK_NULL_HANDLE, &_currentSwapchainImgID);
}

void VulkanRenderer::endRender(uint32_t frameID)
{
	std::vector<VkSwapchainKHR> swapchains = {_swapchain.get()};
	std::vector<uint32_t> imgIds = {_currentSwapchainImgID};
	std::vector<VkResult> results(1);

	std::vector<VkSemaphore> waitSemaphores = {_semRenderDone[frameID]};
	auto info = vkTypes::getPresentInfo(waitSemaphores, swapchains, imgIds, results);

	vkQueuePresentKHR(_mainDevice.getQueueByType(VK_QUEUE_GRAPHICS_BIT), &info);
}

void VulkanRenderer::stop()
{
	window.Close();
	_keepGoing = false;
}

void VulkanRenderer::_initSync()
{
	auto fenceInfo = vkTypes::getFenceCreateInfo();
	auto semaphoreInfo = vkTypes::getSemaphoreCreateInfo();

	_semImgAvailable.resize(_swapchain.getImgCount());
	_semRenderDone.resize(_swapchain.getImgCount());
	_frameFences.resize(_swapchain.getImgCount());
	_imgsInFlight.resize(_swapchain.getImgCount(), false);
	for (uint32_t i = 0; i < _swapchain.getImgCount(); ++i)
	{
		vkCreateSemaphore(_mainDevice.get(), &semaphoreInfo, nullptr, &_semImgAvailable[i]);
		vkCreateSemaphore(_mainDevice.get(), &semaphoreInfo, nullptr, &_semRenderDone[i]);
		vkCreateFence(_mainDevice.get(), &fenceInfo, nullptr, &_frameFences[i]);
	}
}

void VulkanRenderer::_initShaders()
{
	_shaderManager.addShader(_mainDevice, ShaderType::VERTEX, "square");
	_shaderManager.addShader(_mainDevice, ShaderType::FRAGMENT, "ray");
}

void VulkanRenderer::_clearEnv()
{
	_mainDevice.freeCommand(_commandBuffs.data(), _swapchain.getImgCount(), _mainDevice.getQFIdByType(VK_QUEUE_GRAPHICS_BIT));
	_pipeline.destroy(_mainDevice);

	vkQueueWaitIdle(_vulkan.getDevice().getQueueByType(VK_QUEUE_GRAPHICS_BIT));

	_frameBuffs.destroy(_mainDevice);
	_renderPass.destroy(_mainDevice);
	_dsImg.destroy(_mainDevice);
	_swapchain.destroy(_mainDevice);
}

void VulkanRenderer::_initEnv()
{
	if (_firstInit)
	{
		_game.init(&window);
		_geomBuffs.init(_mainDevice);
		_initShaders();
		_descrPool.initLayouts(_mainDevice, _game.getShaderData());
	}

	_surface.initFormat(_mainDevice);
	_swapchain.init(_mainDevice, _surface, window.getSize());
	_dsImg.init(_mainDevice, _surface);
	
	auto shaderInfos = _shaderManager.getShaderStageCreateInfos({{ShaderType::VERTEX, "square"}, {ShaderType::FRAGMENT, "ray"}});
	
	_renderPass.init(_mainDevice, _dsImg.getFormat(), _surface.getFormat().format);
	_pipeline.init(_mainDevice, _renderPass, window.getRenderArea(), _descrPool.getLayouts(), shaderInfos);
	_frameBuffs.init(_mainDevice, _renderPass, window.getSize(), _dsImg.getImgView(), _swapchain.getImgViews());
	_commandBuffs.init(_mainDevice, _swapchain.getImgCount());

	if (_firstInit)
	{
		_descrPool.init(_mainDevice, _swapchain);
		_initSync();
	}
	_firstInit = false;
}

void VulkanRenderer::_recreateEnv()
{
	vkDeviceWaitIdle(_mainDevice.get());

	_clearEnv();
	_initEnv();
}

VkFramebuffer& VulkanRenderer::_getCurrentFrameBuffer()
{
	return _frameBuffs[_currentSwapchainImgID];
}

