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
	_initSurface();
}

VulkanRenderer::~VulkanRenderer()
{
	auto device = _vulkan.getDevice().getDevice();

	for (uint32_t i = 0; i < _swapchainImgCount; ++i)
	{
		vkDestroySemaphore(_mainDevice.getDevice(), _semImgAvailable[i], nullptr);
		vkDestroySemaphore(_mainDevice.getDevice(), _semRenderDone[i], nullptr);
		vkDestroyFence(_mainDevice.getDevice(), _frameFences[i], nullptr);
	}

	_fragmentShader.destroy(device);
	_vertexShader.destroy(device);

	vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);

	_clearEnv();

	vkDestroySurfaceKHR(_vulkan.getInstace(), _surface, nullptr);
}

void VulkanRenderer::init()
{
	std::vector<uint32_t> queueFamilies = { VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT };
	std::vector<VkPhysicalDeviceType> deviceTypesByPriority = { VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU };

	_vulkan.chooseDevice(queueFamilies, deviceTypesByPriority, _surface);

	_mainDevice = _vulkan.getDevice();

	if (_mainDevice.getDevice() == VK_NULL_HANDLE)
		std::cout << std::endl << "No device was chosen." << std::endl;
	else
	{
		std::cout << std::endl << "Device was chosen: " << _vulkan.getPhysDevice().getProps().deviceName << std::endl;

		_initShaders();

		_initDescriptorSetLayout();

		_initEnv();

		// creating vertex buffer 
		std::vector<Vertex> vertices = {
			{{-0.7f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.7f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.7f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.7f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};
		_initStageBuffer(vertices.data(), sizeof(Vertex), vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, _vertexBuff);

		// creating index buffer 
		std::vector<uint16_t> indices = {0,1,2,2,3,0};
		_initStageBuffer(indices.data(), sizeof(uint16_t), indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, _indexBuff);

		// creating uniform buffers
		_uniformBuffs.resize(_swapchainImgCount);
		for (uint32_t i = 0; i < _swapchainImgCount; ++i)
			_initHostBuffer(&_shaderInfo, sizeof(_shaderInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, _uniformBuffs[i]);

		_initDescriptorPool();
		_initDescriptorSets();

		_initSync();

		_currentSwapchainImgID = -1;
	}
}

void VulkanRenderer::run()
{
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
			vkWaitForFences(_mainDevice.getDevice(), 1, &_frameFences[curFrameID], VK_TRUE, UINT64_MAX);
		}

		_imgsInFlight[curFrameID] = true;
		
		vkBeginCommandBuffer(_commandBufs[curFrameID], &cbBeginInfo);

		rpBeginInfo = vkTypes::getRPBeginInfo(_renderPass, _getCurrentFrameBuffer(), renderArea, clearVals);
		vkCmdBeginRenderPass(_commandBufs[curFrameID], &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_commandBufs[curFrameID], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

		std::vector<VkBuffer> vertexBuffers = { _vertexBuff.getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBufs[curFrameID], 0, 1, vertexBuffers.data(), offsets);

		vkCmdBindIndexBuffer(_commandBufs[curFrameID], _indexBuff.getBuffer(), 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(_commandBufs[curFrameID], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[curFrameID], 0, nullptr);
		vkCmdDrawIndexed(_commandBufs[curFrameID], _indexBuff.getElemsCount(), 1, 0, 0, 0);

		vkCmdEndRenderPass(_commandBufs[curFrameID]);

		vkEndCommandBuffer(_commandBufs[curFrameID]);

		vkResetFences(_mainDevice.getDevice(), 1, &_frameFences[curFrameID]);

		_updateUniformDataForImg(curFrameID);

		std::vector<VkSemaphore> waitSemaphores = {_semImgAvailable[curFrameID]};
		std::vector<VkSemaphore> signalSemaphores = {_semRenderDone[curFrameID]};
		std::vector<VkCommandBuffer> cmdBufs = { _commandBufs[curFrameID] };
		VkSubmitInfo submitInfo = vkTypes::getSubmitInfo(waitSemaphores, signalSemaphores, cmdBufs, waitStages);
		vkQueueSubmit(queue, 1, &submitInfo, _frameFences[curFrameID]);

		endRender(curFrameID);

		curFrameID = (curFrameID + 1) % _swapchainImgCount;

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
	auto& dev = _mainDevice.getDevice();
	vkWaitForFences(dev, 1, &_frameFences[frameID], VK_TRUE, UINT64_MAX);
	vkAcquireNextImageKHR(dev, _swapchain, UINT64_MAX, _semImgAvailable[frameID], VK_NULL_HANDLE, &_currentSwapchainImgID);
}

void VulkanRenderer::endRender(uint32_t frameID)
{
	std::vector<VkSwapchainKHR> swapchains = {_swapchain};
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

void VulkanRenderer::_initCommandBuffers()
{
	_commandBufs.resize(_swapchainImgCount);
	_mainDevice.getCommand(_commandBufs.data(), _swapchainImgCount, _mainDevice.getQFIdByType(VK_QUEUE_GRAPHICS_BIT));
}

void VulkanRenderer::_initDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding };
	VkDescriptorSetLayoutCreateInfo info = vkTypes::getDSLCreateInfo(bindings);

	vkCreateDescriptorSetLayout(_mainDevice.getDevice(), &info, nullptr, &_descriptorSetLayout);
}

void VulkanRenderer::_initDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(_swapchainImgCount);

	std::vector<VkDescriptorPoolSize> sizes = {poolSize};
	auto info = vkTypes::getDescriptorPoolCreateInfo(sizes, _swapchainImgCount);

	vkCreateDescriptorPool(_mainDevice.getDevice(), &info, nullptr, &_descriptorPool);
}

void VulkanRenderer::_initDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts = {_descriptorSetLayout, _descriptorSetLayout};
	auto allocInfo = vkTypes::getDescriptorSetAllocateInfo(_descriptorPool, layouts, _swapchainImgCount);

	_descriptorSets.resize(_swapchainImgCount);
	vkAllocateDescriptorSets(_mainDevice.getDevice(), &allocInfo, _descriptorSets.data());

	for (size_t i = 0; i < _swapchainImgCount; i++) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = _uniformBuffs[i].getBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range  = sizeof(ShaderInfoPackage);

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet          = _descriptorSets[i];
		descriptorWrite.dstBinding      = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo     = &bufferInfo;

		vkUpdateDescriptorSets(_mainDevice.getDevice(), 1, &descriptorWrite, 0, nullptr);
	}
}

void VulkanRenderer::_initStageBuffer(void* data, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage, VulkanBuffer& buf)
{
	auto& device = _mainDevice.getDevice();

	uint32_t totalSize = elemSz * nElems;

	VulkanBuffer stagingBuf;
	auto stagingBufType = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBuf.create(_mainDevice, elemSz, nElems, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufType);
	stagingBuf.setData(data, 0, nElems);

	auto vertexBufUse = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
	auto vertexBufType = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	buf.create(_mainDevice, elemSz, nElems, vertexBufUse, vertexBufType);

	stagingBuf.copyTo(buf);
}

void VulkanRenderer::_initHostBuffer(void* data, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage, VulkanBuffer& buf)
{
	auto props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	buf.create(_mainDevice, elemSz, nElems, usage, props);
}

void VulkanRenderer::_initSync()
{
	auto fenceInfo = vkTypes::getFenceCreateInfo();
	auto semaphoreInfo = vkTypes::getSemaphoreCreateInfo();

	_semImgAvailable.resize(_swapchainImgCount);
	_semRenderDone.resize(_swapchainImgCount);
	_frameFences.resize(_swapchainImgCount);
	_imgsInFlight.resize(_swapchainImgCount, false);
	for (uint32_t i = 0; i < _swapchainImgCount; ++i)
	{
		vkCreateSemaphore(_mainDevice.getDevice(), &semaphoreInfo, nullptr, &_semImgAvailable[i]);
		vkCreateSemaphore(_mainDevice.getDevice(), &semaphoreInfo, nullptr, &_semRenderDone[i]);
		vkCreateFence(_mainDevice.getDevice(), &fenceInfo, nullptr, &_frameFences[i]);
	}
}

void VulkanRenderer::_initPipeline()
{
	auto vertexBindingsDescr	= Vertex::getBindingDescriptions();
	auto vertexAttribDescr		= Vertex::getAttributeDescriptions();
	auto vertexISCreateInfo		= vkTypes::getPipelineVertexISCreateInfo(vertexBindingsDescr, vertexAttribDescr);
	
	auto inputAssemblyCreateInfo = vkTypes::getPipelineInputAssemblyISCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	auto renderArea = window.getRenderArea();

	VkViewport viewport{};
	viewport.x		  = 0.0f;
	viewport.y		  = 0.0f;
	viewport.width	  = renderArea.extent.width;
	viewport.height	  = renderArea.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	std::vector<VkViewport> viewports = { viewport };
	std::vector<VkRect2D> scissors = { window.getRenderArea() };
	auto viewportInfo	   = vkTypes::getPipelineViewportSCreateInfo(viewports, scissors);
	auto rasterizationInfo = vkTypes::getPipelineRasterizationSCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, 1.0f, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	auto multisampleInfo   = vkTypes::getPipelineMultisampleSCreateInfo();

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask		 =	VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable		 =	VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor =	VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor =	VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp		 =	VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor =	VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor =	VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp		 =	VK_BLEND_OP_ADD;
	
	// for alpha
	//colorBlendAttachment.blendEnable		 =	VK_TRUE;
	//colorBlendAttachment.srcColorBlendFactor =	VK_BLEND_FACTOR_SRC_ALPHA;
	//colorBlendAttachment.dstColorBlendFactor =	VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//colorBlendAttachment.colorBlendOp		 =	VK_BLEND_OP_ADD;
	//colorBlendAttachment.srcAlphaBlendFactor =	VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment.dstAlphaBlendFactor =	VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment.alphaBlendOp		 =	VK_BLEND_OP_ADD;

	std::vector<VkPipelineColorBlendAttachmentState> attachments = { colorBlendAttachment };
	
	auto colorBlendInfo	  = vkTypes::getPipelineColorBlendSCreateInfo(attachments);
	auto depthStencilInfo = vkTypes::getPipelineDepthStencilSCreateInfo();

	auto& device = _mainDevice.getDevice();

	std::vector<VkDescriptorSetLayout> dsls = { _descriptorSetLayout };
	auto layoutInfo = vkTypes::getPipelineLayoutCreateInfo(dsls, {});
	vkCreatePipelineLayout(_mainDevice.getDevice(), &layoutInfo, nullptr, &_pipelineLayout);

	std::vector<VkPipelineShaderStageCreateInfo> shaderInfos = { _vertexShader.getShaderStageCreateInfo(), _fragmentShader.getShaderStageCreateInfo() };

	auto pipelineInfo = vkTypes::getGraphicsPipelineCreateInfo(_pipelineLayout, _renderPass, 0, shaderInfos, 
		vertexISCreateInfo, inputAssemblyCreateInfo, viewportInfo, rasterizationInfo, multisampleInfo, colorBlendInfo, &depthStencilInfo);
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline);
}

void VulkanRenderer::_initShaders()
{
	auto& dev = _mainDevice.getDevice();
	_vertexShader = VulkanShader("triangle.vert", VK_SHADER_STAGE_VERTEX_BIT);
	_fragmentShader = VulkanShader("triangle.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
	_vertexShader.compile();
	_fragmentShader.compile();
	_vertexShader.create(dev);
	_fragmentShader.create(dev);
}

void VulkanRenderer::_setSurfaceFormat()
{
	auto physDev = _mainDevice.getPhysicalDevice()->getDevice();
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDev, _surface, &_surfaceCapabs);
	//if (_surfaceCapabs.currentExtent.width < UINT32_MAX)
	//	window.setSurfaceSize(_surfaceCapabs.currentExtent.width, _surfaceCapabs.currentExtent.height);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, _surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, _surface, &formatCount, formats.data());
	_surfaceFormat = formats[0];
}

void VulkanRenderer::_clearEnv()
{
	auto& device = _mainDevice.getDevice();

	_mainDevice.freeCommand(_commandBufs.data(), _swapchainImgCount, _mainDevice.getQFIdByType(VK_QUEUE_GRAPHICS_BIT));

	vkDestroyPipeline(device, _pipeline, nullptr);
	vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);

	vkQueueWaitIdle(_vulkan.getDevice().getQueueByType(VK_QUEUE_GRAPHICS_BIT));

	for (uint32_t i = 0; i < _swapchainImgCount; ++i)
		vkDestroyFramebuffer(device, _frameBuffs[i], nullptr);

	vkDestroyRenderPass(device, _renderPass, nullptr);
	vkDestroyImageView(device, _depthStencilImgView, nullptr);
	vkFreeMemory(device, _depthStencilImgMem, nullptr);
	vkDestroyImage(device, _depthStencilImg, nullptr);

	for (uint32_t i = 0; i < _swapchainImgCount; ++i)
		vkDestroyImageView(device, _swapchainImgViews[i], nullptr);
	vkDestroySwapchainKHR(device, _swapchain, nullptr);
}

void VulkanRenderer::_initEnv()
{
	_setSurfaceFormat();

	_initSwapchain();
	_initSwapchainImages();
	_initDepthStencilImage();
	_initRenderPass();
	_initPipeline();
	_initFrameBuffers();
	_initCommandBuffers();
}

void VulkanRenderer::_recreateEnv()
{
	vkDeviceWaitIdle(_mainDevice.getDevice());

	_clearEnv();
	_initEnv();
}

void VulkanRenderer::_updateUniformDataForImg(uint32_t idx)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto renderArea = window.getRenderArea();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	_shaderInfo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	_shaderInfo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	_shaderInfo.proj = glm::perspective(glm::radians(45.0f), renderArea.extent.width / (float)renderArea.extent.height, 0.1f, 10.0f);
	_shaderInfo.proj[1][1] *= -1;

	_uniformBuffs[idx].setData(&_shaderInfo, 0, 1);
}

VkFramebuffer& VulkanRenderer::_getCurrentFrameBuffer()
{
	return _frameBuffs[_currentSwapchainImgID];
}

uint32_t VulkanRenderer::_getMemoryId(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags reqFlags)
{
	auto devMemProps = _mainDevice.getPhysicalDevice()->getMemProps();
	for (uint32_t i = 0; i < devMemProps.memoryTypeCount; ++i)
		if (memReq.memoryTypeBits & (1 << i))
			if ((devMemProps.memoryTypes[i].propertyFlags & reqFlags) == reqFlags)
				return i;
	return -1;
}

