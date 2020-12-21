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

	_surface.destroy(_vulkan);
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

	uint32_t frameID = 0;

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

		beginRender(frameID);

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
		vkCmdBindDescriptorSets(_commandBuffs[frameID], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.getLayout(), 0, 3, sets.data(), 0, nullptr);
		vkCmdDrawIndexed(_commandBuffs[frameID], _geomBuffs.getIndicesCount(), 1, 0, 0, 0);

		vkCmdEndRenderPass(_commandBuffs[frameID]);

		vkEndCommandBuffer(_commandBuffs[frameID]);

		vkResetFences(_mainDevice.get(), 1, &_frameFences[frameID]);

		_game.update(_descrPool, frameID);

		std::vector<VkSemaphore> waitSemaphores = {_semImgAvailable[frameID]};
		std::vector<VkSemaphore> signalSemaphores = {_semRenderDone[frameID]};
		std::vector<VkCommandBuffer> cmdBufs = { _commandBuffs[frameID] };
		VkSubmitInfo submitInfo = vkTypes::getSubmitInfo(waitSemaphores, signalSemaphores, cmdBufs, waitStages);
		vkQueueSubmit(queue, 1, &submitInfo, _frameFences[frameID]);

		endRender(frameID);

		frameID = (frameID + 1) % _swapchain.getImgCount();

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

	auto renderArea = window.getRenderArea();
	_transitionImageLayout(_images[frameID], _surface.getFormat().format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	_copyImageToBuffer(_images[frameID], _images.getBuf(frameID), renderArea.extent.width, renderArea.extent.height);
	_transitionImageLayout(_swapchain.getImgs()[frameID], _surface.getFormat().format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	_scaledImageToImage(_images[frameID], _swapchain.getImgs()[frameID], renderArea.extent.width, renderArea.extent.height, window.getRenderScale());
	_transitionImageLayout(_swapchain.getImgs()[frameID], _surface.getFormat().format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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
	_images.destroy(_mainDevice);
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
	_swapchain.init(_mainDevice, _surface, window.getSizeScaled(), window.getRenderScale());
	_images.init(_mainDevice, _surface, _swapchain, window.getSizeScaled(), window.getRenderScale());
	_dsImg.init(_mainDevice, _surface);
	
	auto shaderInfos = _shaderManager.getShaderStageCreateInfos({{ShaderType::VERTEX, "square"}, {ShaderType::FRAGMENT, "ray"}});
	
	_renderPass.init(_mainDevice, _dsImg.getFormat(), _surface.getFormat().format);
	_pipeline.init(_mainDevice, _renderPass, window.getRenderAreaScaled(), _descrPool.getLayouts(), shaderInfos);
	_frameBuffs.init(_mainDevice, _renderPass, window.getSizeScaled(), _dsImg.getImgView(), _images.getImgViews());
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

VkCommandBuffer VulkanRenderer::_beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _mainDevice.getPool(_mainDevice.getQFIdByType(VK_QUEUE_GRAPHICS_BIT));
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_mainDevice.get(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanRenderer::_endSingleTimeCommands(const VkCommandBuffer& commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	auto graphicsQueue = _mainDevice.getQueueByType(VK_QUEUE_GRAPHICS_BIT);
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(_mainDevice.get(), _mainDevice.getPool(_mainDevice.getQFIdByType(VK_QUEUE_GRAPHICS_BIT)), 1, &commandBuffer);
}

void VulkanRenderer::_transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

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
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	_endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::_copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	_endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::_copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyImageToBuffer(
		commandBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		buffer,
		1,
		&region
	);

	_endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::_scaledImageToImage(VkImage image1, VkImage image2, uint32_t width, uint32_t height, float scale) {
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

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
	vkCmdBlitImage(commandBuffer, image1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image2, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlitRegion, VK_FILTER_NEAREST);

	_endSingleTimeCommands(commandBuffer);
}
