#include "VulkanRenderer.h"
#include "Window.h"
#include "VulkanBuffer.h"

#include "VulkanErrorChecker.h"

#include "FPSCounter.h"

#include <iostream>

Window window(512, 512, L"test");

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
	_initSurface();
}

VulkanRenderer::~VulkanRenderer()
{
	auto device = _vulkan.getDevice().getDevice();

	_fragmentShader.destroy(device);
	_vertexShader.destroy(device);

	vkQueueWaitIdle(_vulkan.getDevice().getQueueByType(VK_QUEUE_GRAPHICS_BIT));

	vkDestroyFence(device, _swapchainImgAvailable, nullptr);

	for (uint32_t i = 0; i < _swapchainImgCount; ++i)
		vkDestroyFramebuffer(device, _frameBuffs[i], nullptr);
	
	vkDestroyRenderPass(device, _renderPass, nullptr);

	vkDestroyImageView(device, _depthStencilImgView, nullptr);
	vkFreeMemory(device, _depthStencilImgMem, nullptr);
	vkDestroyImage(device, _depthStencilImg, nullptr);

	for (uint32_t i = 0; i < _swapchainImgCount; ++i)
		vkDestroyImageView(device, _swapchainImgViews[i], nullptr);
	vkDestroySwapchainKHR(device, _swapchain, nullptr);
	vkDestroySurfaceKHR(_vulkan.getInstace(), _surface, nullptr);
}

void VulkanRenderer::init()
{
	std::vector<uint32_t> queueFamilies = { VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT };
	std::vector<VkPhysicalDeviceType> deviceTypesByPriority = { VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU };

	_vulkan.chooseDevice(queueFamilies, deviceTypesByPriority, _surface);

	_mainDevice = _vulkan.getDevice();

	if (_mainDevice.getDevice() == VK_NULL_HANDLE)
		std::cout << std::endl << "No device was chosen." << std::endl;
	else
	{
		std::cout << std::endl << "Device was chosen: " << _vulkan.getPhysDevice().getProps().deviceName << std::endl;

		_setSurfaceFormat();
		_initSwapchain();
		_initSwapchainImages();
		_initDepthStencilImage();
		_initShaders();
		_initRenderPass();
		_initFrameBuffers();

		_initSync();


		_currentSwapchainImgID = -1;
	}
}

void VulkanRenderer::run()
{
	auto queue = _mainDevice.getQueueByType(VK_QUEUE_GRAPHICS_BIT);

	VkCommandBuffer commands;
	_mainDevice.getCommand(&commands, 1, _mainDevice.getQFIdByType(VK_QUEUE_GRAPHICS_BIT));

	auto cbBeginInfo = vkTypes::getCBBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkRect2D renderArea;
	renderArea.offset = { 0,0 };
	auto wSz = window.getSize();
	renderArea.extent = {wSz.first, wSz.second};
	std::vector<VkClearValue> clearVals(2);
	clearVals[0].depthStencil.depth = 0.0f;
	clearVals[0].depthStencil.stencil = 0;
	clearVals[1].color.float32[0] = 1.0f;
	clearVals[1].color.float32[1] = 0.0f;
	clearVals[1].color.float32[2] = 0.0f;
	clearVals[1].color.float32[3] = 0.0f;

	VkRenderPassBeginInfo rpBeginInfo;

	auto semaphoreInfo = vkTypes::getSemaphoreCreateInfo();
	std::vector<VkSemaphore> semaphores(1);
	auto& renderCompleteSemaphore = semaphores[0];
	vkCreateSemaphore(_mainDevice.getDevice(), &semaphoreInfo, nullptr, &renderCompleteSemaphore);

	std::vector<VkSemaphore> waitSemaphores;
	std::vector<VkSemaphore> signalSemaphores = { renderCompleteSemaphore };
	std::vector<VkCommandBuffer> cmdBuffs = { commands };
	VkSubmitInfo submitInfo = vkTypes::getSubmitInfo(waitSemaphores, signalSemaphores, cmdBuffs, nullptr);
	
	FPSCounter fpsCounter;

	while (runOnce())
	{
		fpsCounter.tellFPS(1000);

		beginRender();
		
		vkBeginCommandBuffer(commands, &cbBeginInfo);

		rpBeginInfo = vkTypes::getRPBeginInfo(_renderPass, _getCurrentFrameBuffer(), renderArea, clearVals);
		vkCmdBeginRenderPass(commands, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdEndRenderPass(commands);

		vkEndCommandBuffer(commands);

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

		endRender(semaphores);
	}

	vkQueueWaitIdle(queue);

	vkDestroySemaphore(_mainDevice.getDevice(), renderCompleteSemaphore, nullptr);
}

bool VulkanRenderer::runOnce()
{
	return window.Update();
}

void VulkanRenderer::beginRender()
{
	auto& dev = _mainDevice.getDevice();
	vkAcquireNextImageKHR(dev, _swapchain, UINT64_MAX, VK_NULL_HANDLE, _swapchainImgAvailable, &_currentSwapchainImgID);
	vkWaitForFences(dev, 1, &_swapchainImgAvailable, VK_TRUE, UINT64_MAX);
	vkResetFences(dev, 1, &_swapchainImgAvailable);
	vkQueueWaitIdle(_mainDevice.getQueueByType(VK_QUEUE_GRAPHICS_BIT));
}

void VulkanRenderer::endRender(const std::vector<VkSemaphore>& waitSemaphores)
{
	std::vector<VkSwapchainKHR> swapchains = {_swapchain};
	std::vector<uint32_t> imgIds = {_currentSwapchainImgID};
	std::vector<VkResult> results(1);

	auto info = vkTypes::getPresentInfo(waitSemaphores, swapchains, imgIds, results);

	vkQueuePresentKHR(_mainDevice.getQueueByType(VK_QUEUE_COMPUTE_BIT), &info);
}

void VulkanRenderer::stop()
{
	window.Close();
	_keepGoing = false;
}

void VulkanRenderer::_initSurface()
{
	auto info = vkTypes::getSurfaceCreateInfo(window.getHInstance(), window.getHWND());
	vkCreateWin32SurfaceKHR(_vulkan.getInstace(), &info, nullptr, &_surface);
}

void VulkanRenderer::_initSwapchain()
{
	uint32_t bufferSz = 2;
	if (bufferSz < _surfaceCapabs.minImageCount) 
		bufferSz = _surfaceCapabs.minImageCount;
	else if (_surfaceCapabs.maxImageCount > 0 && bufferSz > _surfaceCapabs.maxImageCount) 
		bufferSz = _surfaceCapabs.maxImageCount;

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t nPresentModes = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_vulkan.getPhysDevice().getDevice(), _surface, &nPresentModes, nullptr);
		std::vector<VkPresentModeKHR> presentModes(nPresentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_vulkan.getPhysDevice().getDevice(), _surface, &nPresentModes, presentModes.data());
		for (auto pm : presentModes)
			if (pm == VK_PRESENT_MODE_MAILBOX_KHR) 
				present_mode = pm;
	}

	auto info = vkTypes::getSwapchainCreateInfo(_surface, _surfaceFormat, present_mode, bufferSz, window.getSize().first, window.getSize().second);

	vkCreateSwapchainKHR(_vulkan.getDevice().getDevice(), &info, nullptr, &_swapchain);
	vkGetSwapchainImagesKHR(_vulkan.getDevice().getDevice(), _swapchain, &_swapchainImgCount, nullptr);
}

void VulkanRenderer::_initSwapchainImages()
{
	_swapchainImgs.resize(_swapchainImgCount);
	_swapchainImgViews.resize(_swapchainImgCount);

	vkGetSwapchainImagesKHR(_vulkan.getDevice().getDevice(), _swapchain, &_swapchainImgCount, _swapchainImgs.data());

	auto rgba = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentMapping mapping = { rgba, rgba, rgba, rgba };
	VkImageSubresourceRange subRng;
	subRng.aspectMask	  = VK_IMAGE_ASPECT_COLOR_BIT;
	subRng.baseMipLevel	  = 0;
	subRng.levelCount	  = 1;
	subRng.baseArrayLayer = 0;
	subRng.layerCount	  = 1;

	for (uint32_t i = 0; i < _swapchainImgCount; ++i)
	{
		auto info = vkTypes::getImageViewCreateInfo(_swapchainImgs[i], mapping, subRng, _surfaceFormat.format, VK_IMAGE_VIEW_TYPE_2D);
		vkCreateImageView(_vulkan.getDevice().getDevice(), &info, nullptr, &_swapchainImgViews[i]);
	}
}

void VulkanRenderer::_initDepthStencilImage()
{
	std::vector<VkFormat> formatsToTry =
	{
		//VK_FORMAT_D32_SFLOAT,
		//VK_FORMAT_D16_UNORM,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_X8_D24_UNORM_PACK32,
		VK_FORMAT_S8_UINT
	};

	_depthStencilFormat = VK_FORMAT_UNDEFINED;
	VkFormatProperties fProps;
	for (auto& format : formatsToTry)
	{
		vkGetPhysicalDeviceFormatProperties(_mainDevice.getPhysicalDevice()->getDevice(), format, &fProps);
		if (fProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			_depthStencilFormat = format;
			break;
		}
	}
	
	auto imgInfo = vkTypes::getImageCreateInfo(VK_IMAGE_TYPE_2D, _depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		_surfaceCapabs.currentExtent.width, _surfaceCapabs.currentExtent.height);

	vkCreateImage(_mainDevice.getDevice(), &imgInfo, nullptr, &_depthStencilImg);

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(_mainDevice.getDevice(), _depthStencilImg, &memReq);

	uint32_t memId = _getMemoryId(memReq, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	auto allocInfo = vkTypes::getMemAllocInfo(memReq.size, memId);
	vkAllocateMemory(_mainDevice.getDevice(), &allocInfo, nullptr, &_depthStencilImgMem);
	vkBindImageMemory(_mainDevice.getDevice(), _depthStencilImg, _depthStencilImgMem, 0);

	auto rgba = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentMapping mapping = { rgba, rgba, rgba, rgba };
	VkImageSubresourceRange subRng;
	subRng.aspectMask	  = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	subRng.baseMipLevel   = 0;
	subRng.levelCount	  = 1;
	subRng.baseArrayLayer = 0;
	subRng.layerCount	  = 1;

	auto viewInfo = vkTypes::getImageViewCreateInfo(_depthStencilImg, mapping, subRng, _depthStencilFormat, VK_IMAGE_VIEW_TYPE_2D);

	vkCreateImageView(_mainDevice.getDevice(), &viewInfo, nullptr, &_depthStencilImgView);
}

void VulkanRenderer::_initRenderPass()
{
	std::vector<VkAttachmentDescription> attachments(2);
	attachments[0].flags          =		0;
	attachments[0].format         =		_depthStencilFormat;
	attachments[0].samples        =		VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp         =		VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp        =		VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].stencilLoadOp  =		VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp =		VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout  =		VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout	  =		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachments[1].flags		 =	0;
	attachments[1].format		 =	_surfaceFormat.format;
	attachments[1].samples		 =	VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp		 =	VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp		 =	VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].initialLayout =	VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout	 =	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkAttachmentReference> colorAttachments(1);
	colorAttachments[0].attachment = 1;
	colorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription> subpasses(1);
	subpasses[0].pipelineBindPoint		 =	VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount	 =	colorAttachments.size();
	subpasses[0].pColorAttachments		 =	colorAttachments.data();		// layout(location=0) out vec4 FinalColor;
	subpasses[0].pDepthStencilAttachment =	&depthAttachmentRef;

	auto info = vkTypes::getRPCreateInfo(attachments, subpasses);

	vkCreateRenderPass(_mainDevice.getDevice(), &info, nullptr, &_renderPass);
}

void VulkanRenderer::_initFrameBuffers()
{
	_frameBuffs.resize(_swapchainImgCount);

	std::vector<VkImageView> attachments(2);
	attachments[0] = _depthStencilImgView;

	for (uint32_t i = 0; i < _swapchainImgCount; ++i)
	{
		attachments[1] = _swapchainImgViews[i];
		auto info = vkTypes::getFramebufferCreateInfo(_renderPass, attachments, window.getSize().first, window.getSize().second, 1);

		vkCreateFramebuffer(_mainDevice.getDevice(), &info, nullptr, &_frameBuffs[i]);
	}
}

void VulkanRenderer::_initSync()
{
	auto info = vkTypes::getFenceCreateInfo();

	vkCreateFence(_mainDevice.getDevice(), &info, nullptr, &_swapchainImgAvailable);
}

void VulkanRenderer::_initShaders()
{
	auto& dev = _mainDevice.getDevice();
	_vertexShader = VulkanShader(dev, "vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	_fragmentShader = VulkanShader(dev, "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void VulkanRenderer::_setSurfaceFormat()
{
	auto physDev = _mainDevice.getPhysicalDevice()->getDevice();
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDev, _surface, &_surfaceCapabs);
	if (_surfaceCapabs.currentExtent.width < UINT32_MAX)
		window.setSurfaceSize(_surfaceCapabs.currentExtent.width, _surfaceCapabs.currentExtent.height);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, _surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, _surface, &formatCount, formats.data());
	_surfaceFormat = formats[0];
}

VkFramebuffer& VulkanRenderer::_getCurrentFrameBuffer()
{
	return _frameBuffs[_currentSwapchainImgID];
}

uint32_t VulkanRenderer::_getMemoryId(const VkMemoryRequirements& memReq, VkMemoryPropertyFlagBits reqFlags)
{
	auto devMemProps = _mainDevice.getPhysicalDevice()->getMemProps();
	for (uint32_t i = 0; i < devMemProps.memoryTypeCount; ++i)
		if (memReq.memoryTypeBits & (1 << i))
			if ((devMemProps.memoryTypes[i].propertyFlags & reqFlags) == reqFlags)
				return i;
	return -1;
}

