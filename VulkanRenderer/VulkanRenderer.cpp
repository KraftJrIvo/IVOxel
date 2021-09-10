#include "VulkanRenderer.h"
#include "Vertex.h"

#include "FPSCounter.h"
#include "FPSLimiter.h"

#include <iostream>
#include <thread>
#include <mutex>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#define FPS_LIMIT 144

VulkanRenderer::VulkanRenderer(Window& w, GameState& gs) :
	AbstractRenderer(w, gs, true),
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
	_surface.init(_vulkan, _window);

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

	_surface.destroy(_vulkan);
}

void VulkanRenderer::startRender()
{
	auto& cam = _gs.getCam();
	auto& map = _gs.getMap();

	auto queue = _mainDevice.getQueueByType(VK_QUEUE_GRAPHICS_BIT);

	auto cbBeginInfo = vkTypes::getCBBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	auto ra = _window.getRenderArea();
	VkRect2D renderArea = { ra.x, ra.y, ra.z, ra.w };
	std::vector<VkClearValue> clearVals(2);
	clearVals[0].depthStencil.depth = 1.0f;
	clearVals[0].depthStencil.stencil = 0;
	clearVals[1].color.float32[0] = 1.0f;
	clearVals[1].color.float32[1] = 0.0f;
	clearVals[1].color.float32[2] = 0.0f;
	clearVals[1].color.float32[3] = 0.0f;

	VkRenderPassBeginInfo rpBeginInfo;

	FPSCounter fpsCounter;
	FPSLimiter fpsLimiter(FPS_LIMIT);
	fpsLimiter.reset();

	uint32_t frameID = 0;
	uint32_t counter = 0;

	static std::thread t([&]() {_gs.startUpdateLoop(&_descrPool, _swapchain.getImgCount()); });

	while (_runOnce())
	{
		fpsCounter.tellFPS(1000);

		if (_window.wasResized())
		{
			do
			{
				ra = _window.getRenderArea();
				renderArea.offset.x = ra.x; renderArea.offset.y = ra.y; renderArea.extent.width = ra.z; renderArea.extent.height = ra.w;
				cam.res = { renderArea.extent.width, renderArea.extent.height };
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				_window.Update();
			} 
			while (!renderArea.extent.width || !renderArea.extent.height);
			_recreateEnv();
		}

		_beginRender(frameID);

		if (_imgsInFlight[frameID]) {
			vkWaitForFences(_mainDevice.get(), 1, &_frameFences[frameID], VK_TRUE, UINT64_MAX);
		}

		_imgsInFlight[frameID] = true;
		
		vkBeginCommandBuffer(_commandBuffs[frameID], &cbBeginInfo);

		rpBeginInfo = vkTypes::getRPBeginInfo(_renderPass.get(), _getCurrentFrameBuffer(), renderArea, clearVals);
		vkCmdBeginRenderPass(_commandBuffs[frameID], &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_commandBuffs[frameID], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.get());

		_geomBuffs.bindCmdBuffer(_mainDevice, _commandBuffs[frameID]);

		auto& sets = _descrPool.getSets(frameID);
		vkCmdBindDescriptorSets(_commandBuffs[frameID], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
		vkCmdDrawIndexed(_commandBuffs[frameID], _geomBuffs.getIndicesCount(), 1, 0, 0, 0);

		vkCmdEndRenderPass(_commandBuffs[frameID]);

		_transitionImageLayout(_commandBuffs[frameID], _images[frameID], _surface.getFormat().format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		_transitionImageLayout(_commandBuffs[frameID], _swapchain.getImgs()[frameID], _surface.getFormat().format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		_blitImage(_commandBuffs[frameID], _images[frameID], _swapchain.getImgs()[frameID], renderArea.extent.width, renderArea.extent.height, _window.getRenderScale());
		_transitionImageLayout(_commandBuffs[frameID], _swapchain.getImgs()[frameID], _surface.getFormat().format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		vkEndCommandBuffer(_commandBuffs[frameID]);

		{
			std::lock_guard<std::mutex> lock(_gs.updMtx);
			
			vkResetFences(_mainDevice.get(), 1, &_frameFences[frameID]);

			std::vector<VkSemaphore> waitSemaphores = { _semImgAvailable[frameID] };
			std::vector<VkSemaphore> signalSemaphores = { _semRenderDone[frameID] };
			std::vector<VkCommandBuffer> cmdBufs = { _commandBuffs[frameID] };
			VkSubmitInfo submitInfo = vkTypes::getSubmitInfo(waitSemaphores, signalSemaphores, cmdBufs, waitStages);
			vkQueueSubmit(queue, 1, &submitInfo, _frameFences[frameID]);
		}
		
		_endRender(frameID);

		frameID = (frameID + 1) % _swapchain.getImgCount();
		counter++;

		if (FPS_LIMIT)
			fpsLimiter.tick();
	}

	vkQueueWaitIdle(queue);
}

bool VulkanRenderer::_runOnce()
{
	return _window.Update();
}

void VulkanRenderer::_beginRender(uint32_t frameID)
{
	auto& dev = _mainDevice.get();
	vkWaitForFences(dev, 1, &_frameFences[frameID], VK_TRUE, UINT64_MAX);
	vkAcquireNextImageKHR(dev, _swapchain.get(), UINT64_MAX, _semImgAvailable[frameID], VK_NULL_HANDLE, &_currentSwapchainImgID);
}

void VulkanRenderer::_endRender(uint32_t frameID)
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
	_window.Close();
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
	_images.destroy(_mainDevice);
	_swapchain.destroy(_mainDevice);
}

void VulkanRenderer::_initEnv()
{
	if (_firstInit)
	{
		_geomBuffs.init(_mainDevice);
		_initShaders();
		_descrPool.initLayouts(_mainDevice, _gs.getGameData());
	}

	_surface.initFormat(_mainDevice);
	_swapchain.init(_mainDevice, _surface, _window.getSizeScaled(), _window.getRenderScale());
	_images.init(_mainDevice, _surface, _swapchain, _window.getSizeScaled(), _window.getRenderScale());
	_dsImg.init(_mainDevice, _surface);
	
	auto shaderInfos = _shaderManager.getShaderStageCreateInfos({{ShaderType::VERTEX, "square"}, {ShaderType::FRAGMENT, "ray"}});
	
	_renderPass.init(_mainDevice, _dsImg.getFormat(), _surface.getFormat().format);
	auto ra = _window.getRenderAreaScaled();
	VkRect2D r2d; r2d.offset.x = ra.x; r2d.offset.y = ra.y; r2d.extent.width = ra.z; r2d.extent.height = ra.w;
	_pipeline.init(_mainDevice, _renderPass, r2d, _descrPool.getLayouts(), shaderInfos);
	_frameBuffs.init(_mainDevice, _renderPass, _window.getSizeScaled(), _dsImg.getImgView(), _images.getImgViews());
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

void VulkanRenderer::_transitionImageLayout(VkCommandBuffer cb, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
#ifdef _DEBUG
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
		barrier.srcAccessMask = (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) ? VK_ACCESS_TRANSFER_WRITE_BIT : 0;
		barrier.dstAccessMask = (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) ? VK_ACCESS_TRANSFER_WRITE_BIT : 0;

		sourceStage = (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) ? VK_ACCESS_TRANSFER_WRITE_BIT : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
	else if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	vkCmdPipelineBarrier(
		cb,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
#endif
}

void VulkanRenderer::_blitImage(VkCommandBuffer cb, VkImage image1, VkImage image2, uint32_t width, uint32_t height, float scale) {
	VkOffset3D topLeft = { 0, 0, 0 };
	VkOffset3D botRight = { width, height, 1 };
	VkOffset3D topLeftSc = { 0, 0, 0 };
	VkOffset3D botRightSc = { (uint32_t)(width * scale), (uint32_t)(height * scale), 1 };
	VkImageBlit imageBlitRegion{};
	imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlitRegion.srcSubresource.layerCount = 1;
	imageBlitRegion.srcOffsets[0] = topLeft;
	imageBlitRegion.srcOffsets[1] = botRight;
	imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlitRegion.dstSubresource.layerCount = 1;
	imageBlitRegion.dstOffsets[0] = topLeftSc;
	imageBlitRegion.dstOffsets[1] = botRightSc;
	vkCmdBlitImage(cb, image1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image2, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlitRegion, VK_FILTER_NEAREST);
}
