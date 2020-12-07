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

	_fragmentShader.destroy(device);
	_vertexShader.destroy(device);

	vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, _viewShaderInfoDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, _lightingShaderInfoDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, _mapShaderInfoDescriptorSetLayout, nullptr);

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

		_initShaders();

		_initDescriptorSetLayout(_viewShaderInfoDescriptorSetLayout);
		_initDescriptorSetLayout(_lightingShaderInfoDescriptorSetLayout);
		_initDescriptorSetLayout(_mapShaderInfoDescriptorSetLayout);

		_initEnv();

		// creating vertex buffer 
		std::vector<Vertex> vertices = {
			{{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
			{{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
			{{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			{{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
		};
		_initStageBuffer(vertices.data(), sizeof(Vertex), vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, _vertexBuff);

		// creating index buffer 
		std::vector<uint16_t> indices = {0,1,2,2,3,0};
		_initStageBuffer(indices.data(), sizeof(uint16_t), indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, _indexBuff);

		uint32_t align = _mainDevice.getPhysicalDevice()->getProps().limits.minUniformBufferOffsetAlignment;
		uint32_t offset1 = sizeof(ViewShaderInfo) + align - (sizeof(ViewShaderInfo) % align);
		uint32_t offset2 = offset1 + sizeof(LightingShaderInfo) + align - ((offset1 + sizeof(LightingShaderInfo)) % align);

		// creating uniform buffers
		_uniformBuffs.resize(_swapchain.getImgCount());
		for (uint32_t i = 0; i < _swapchain.getImgCount(); ++i)
			_initHostBuffer(offset1 + offset2 + sizeof(MapShaderInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, _uniformBuffs[i]);

		_initDescriptorPool();
		_initDescriptorSet(_viewShaderInfoDescriptorSetLayout, _viewShaderInfoDescriptorSets, sizeof(ViewShaderInfo), 0);
		_initDescriptorSet(_lightingShaderInfoDescriptorSetLayout, _lightingShaderInfoDescriptorSets, sizeof(LightingShaderInfo), offset1);
		_initDescriptorSet(_mapShaderInfoDescriptorSetLayout, _mapShaderInfoDescriptorSets, sizeof(MapShaderInfo), offset2);

		_initSync();

		_curRot = { 0.0f, -90.0f };
		_curTrans = { 0.5f, 0.5f, 0.5f };

		_currentSwapchainImgID = -1;
	}
}

void VulkanRenderer::run(const VoxelMap& map)
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
			vkWaitForFences(_mainDevice.get(), 1, &_frameFences[curFrameID], VK_TRUE, UINT64_MAX);
		}

		_imgsInFlight[curFrameID] = true;
		
		vkBeginCommandBuffer(_commandBuffs[curFrameID], &cbBeginInfo);

		rpBeginInfo = vkTypes::getRPBeginInfo(_renderPass.get(), _getCurrentFrameBuffer(), renderArea, clearVals);
		vkCmdBeginRenderPass(_commandBuffs[curFrameID], &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_commandBuffs[curFrameID], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.get());

		std::vector<VkBuffer> vertexBuffers = { _vertexBuff.getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffs[curFrameID], 0, 1, vertexBuffers.data(), offsets);

		vkCmdBindIndexBuffer(_commandBuffs[curFrameID], _indexBuff.getBuffer(), 0, VK_INDEX_TYPE_UINT16);

		std::vector<VkDescriptorSet> sets = { _viewShaderInfoDescriptorSets[curFrameID], _lightingShaderInfoDescriptorSets[curFrameID], _mapShaderInfoDescriptorSets[curFrameID] };
		vkCmdBindDescriptorSets(_commandBuffs[curFrameID], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.getLayout(), 0, 3, sets.data(), 0, nullptr);
		vkCmdDrawIndexed(_commandBuffs[curFrameID], _indexBuff.getElemsCount(), 1, 0, 0, 0);

		vkCmdEndRenderPass(_commandBuffs[curFrameID]);

		vkEndCommandBuffer(_commandBuffs[curFrameID]);

		vkResetFences(_mainDevice.get(), 1, &_frameFences[curFrameID]);

		_updateViewShaderInfo(curFrameID);
		_updateLightingShaderInfo(map, curFrameID);
		_updateMapShaderInfo(map, curFrameID);

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

void VulkanRenderer::_initDescriptorSetLayout(VkDescriptorSetLayout& layout)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding };
	VkDescriptorSetLayoutCreateInfo info = vkTypes::getDSLCreateInfo(bindings);

	vkCreateDescriptorSetLayout(_mainDevice.get(), &info, nullptr, &layout);
}

void VulkanRenderer::_initDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 3 * static_cast<uint32_t>(_swapchain.getImgCount());

	std::vector<VkDescriptorPoolSize> sizes = {poolSize};
	auto info = vkTypes::getDescriptorPoolCreateInfo(sizes, 3 * _swapchain.getImgCount());

	vkCreateDescriptorPool(_mainDevice.get(), &info, nullptr, &_descriptorPool);
}

void VulkanRenderer::_initDescriptorSet(VkDescriptorSetLayout& layout, std::vector<VkDescriptorSet>& set, uint32_t range, uint32_t offset)
{
	std::vector<VkDescriptorSetLayout> layouts(_swapchain.getImgCount(), layout);
	auto allocInfo = vkTypes::getDescriptorSetAllocateInfo(_descriptorPool, layouts, _swapchain.getImgCount());

	set.resize(_swapchain.getImgCount());
	vkAllocateDescriptorSets(_mainDevice.get(), &allocInfo, set.data());

	for (size_t i = 0; i < _swapchain.getImgCount(); i++) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = _uniformBuffs[i].getBuffer();
		bufferInfo.offset = offset;
		bufferInfo.range  = range;

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet          = set[i];
		descriptorWrite.dstBinding      = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo     = &bufferInfo;

		vkUpdateDescriptorSets(_mainDevice.get(), 1, &descriptorWrite, 0, nullptr);
	}
}

void VulkanRenderer::_initStageBuffer(void* data, uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage, VulkanBuffer& buf)
{
	auto& device = _mainDevice.get();

	uint32_t totalSize = elemSz * nElems;

	VulkanBuffer stagingBuf;
	auto stagingBufType = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBuf.create(_mainDevice, elemSz, nElems, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufType);
	stagingBuf.setData(data, 0, elemSz * nElems);

	auto vertexBufUse = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
	auto vertexBufType = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	buf.create(_mainDevice, elemSz, nElems, vertexBufUse, vertexBufType);

	stagingBuf.copyTo(buf);
}

void VulkanRenderer::_initHostBuffer(uint32_t elemSz, uint32_t nElems, VkBufferUsageFlagBits usage, VulkanBuffer& buf)
{
	auto props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	buf.create(_mainDevice, elemSz, nElems, usage, props);
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
	auto& dev = _mainDevice.get();
	_vertexShader = VulkanShader("square.vert", VK_SHADER_STAGE_VERTEX_BIT);
	_fragmentShader = VulkanShader("ray.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
	_vertexShader.compile();
	_fragmentShader.compile();
	_vertexShader.create(dev);
	_fragmentShader.create(dev);
}

void VulkanRenderer::_clearEnv()
{
	auto& device = _mainDevice.get();

	_mainDevice.freeCommand(_commandBuffs.data(), _swapchain.getImgCount(), _mainDevice.getQFIdByType(VK_QUEUE_GRAPHICS_BIT));

	_pipeline.destroy(_mainDevice);

	vkQueueWaitIdle(_vulkan.getDevice().getQueueByType(VK_QUEUE_GRAPHICS_BIT));

	_frameBuffs.destroy(_mainDevice);
	_renderPass.destroy(_mainDevice);
	_dsImg.destroy(_mainDevice);
	_swapchain.destroy(_mainDevice);
	//_surface.destroy(_vulkan);
}

void VulkanRenderer::_initEnv()
{
	_surface.initFormat(_mainDevice);

	_swapchain.init(_mainDevice, _surface, window.getSize());
	
	_dsImg.init(_mainDevice, _surface);
	
	std::vector<VkDescriptorSetLayout> dsls = { _viewShaderInfoDescriptorSetLayout, _lightingShaderInfoDescriptorSetLayout, _mapShaderInfoDescriptorSetLayout };
	std::vector<VkPipelineShaderStageCreateInfo> shaderInfos = { _vertexShader.getShaderStageCreateInfo(), _fragmentShader.getShaderStageCreateInfo() };
	
	_renderPass.init(_mainDevice, _dsImg.getFormat(), _surface.getFormat().format);
	_pipeline.init(_mainDevice, _renderPass, window.getRenderArea(), dsls, shaderInfos);
	_frameBuffs.init(_mainDevice, _renderPass, window.getSize(), _dsImg.getImgView(), _swapchain.getImgViews());
	_commandBuffs.init(_mainDevice, _swapchain.getImgCount());
}

void VulkanRenderer::_recreateEnv()
{
	vkDeviceWaitIdle(_mainDevice.get());

	_clearEnv();
	_initEnv();
}

void VulkanRenderer::_updateViewShaderInfo(uint32_t idx)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto renderArea = window.getRenderArea();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	auto deltaRot = window.getCurDeltaRot();
	glm::vec2 deltaR = { deltaRot[0], -deltaRot[1] };
	deltaR /= (float)renderArea.extent.width;
	deltaR *= 500.0f;
	_curRot += deltaR;

	auto model = glm::mat4(1.0f);
	//model = glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::rotate(view, glm::radians(_curRot.y), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, glm::radians(_curRot.x), glm::vec3(0.0f, 1.0f, 0.0f));

	auto deltaTrans = window.getCurDeltaTrans();
	glm::vec3 deltaT = { deltaTrans[0], deltaTrans[1], deltaTrans[2] };
	glm::mat4 rotmat(1.0);
	rotmat = glm::rotate(rotmat, glm::radians(-_curRot.y - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotmat = glm::rotate(rotmat, glm::radians(-_curRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
	deltaT = glm::mat3(rotmat) * deltaT;
	deltaT /= 100.0f;
	_curTrans += deltaT;

	auto w = renderArea.extent.width;
	auto h = renderArea.extent.height;
	auto aspect = 1.0f;// w / (float)h;
	auto proj = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 10.0f);
	proj[1][1] *= -1;

	_viewShaderInfo.mvp = proj * view;// proj* view* model;
	_viewShaderInfo.time = time;
	_viewShaderInfo.resolution = {renderArea.extent.width, renderArea.extent.height };
	_viewShaderInfo.fov = 90.0f;
	_viewShaderInfo.pos = _curTrans;

	_uniformBuffs[idx].setData(&_viewShaderInfo, 0, sizeof(_viewShaderInfo));
}

void VulkanRenderer::_updateMapShaderInfo(const VoxelMap& map, uint32_t idx)
{
	int32_t curOffset = 0;
	for (int i = -1; i <= 1; ++i)
		for (int j = -1; j <= 1; ++j)
			for (int k = -1; k <= 1; ++k)
			{
				uint32_t id = (k + 1) * 9 + (j + 1) * 3 + (i + 1);
				uint32_t vecId = id / 4;
				uint32_t elemId = id % 4;
				int32_t test = 100 * (k + 1) + 10 * (j + 1) + (i + 1);
				VoxelChunk* chunk = map.getChunk({ i,j,k });
				if (chunk)
				{
					std::memcpy((char*)_mapShaderInfo.chunkData + curOffset, chunk->pyramid.data.data(), chunk->pyramid.data.size());
					_mapShaderInfo.chunkOffsets[vecId][elemId] = curOffset;
					curOffset += chunk->pyramid.data.size();
				}
				else
				{
					_mapShaderInfo.chunkOffsets[vecId][elemId] = -1;
				}
			}

	uint32_t align = _mainDevice.getPhysicalDevice()->getProps().limits.minUniformBufferOffsetAlignment;
	uint32_t offset1 = sizeof(ViewShaderInfo) + align - (sizeof(ViewShaderInfo) % align);
	uint32_t offset2 = offset1 + sizeof(LightingShaderInfo) + align - ((offset1 + sizeof(LightingShaderInfo)) % align);
	_uniformBuffs[idx].setData(&_mapShaderInfo, offset2, sizeof(_mapShaderInfo));
}

void VulkanRenderer::_updateLightingShaderInfo(const VoxelMap& map, uint32_t idx)
{
	auto lsbc = map.getLightsByChunks();
	uint32_t curLight = 0;
	const size_t colorSz = RGB * sizeof(float);
	const size_t posSz = DIMENSIONS * sizeof(float);
	static float t = 0;
	for (auto& ls : lsbc)
	{
		for (auto& l : ls)
		{
			std::vector<float> color = { l.rgba[R] / 255.0f, l.rgba[G] / 255.0f, l.rgba[B] / 255.0f };

			auto pos = l.position;

			if (&l == &*ls.begin())
			{
				pos[0] = pos[0] + cos(t / 10.);
				pos[1] = pos[1] + sin(t / 5.);
				pos[2] = pos[2] + sin(t / 10.);
			}
			else
			{
				pos[0] = pos[0] + cos(-t);
				pos[2] = pos[2] + sin(-t);
			}

			_lightingShaderInfo.coords[curLight].x = pos[0];
			_lightingShaderInfo.coords[curLight].y = pos[1];
			_lightingShaderInfo.coords[curLight].z = pos[2];
			_lightingShaderInfo.colors[curLight].x = color[0];
			_lightingShaderInfo.colors[curLight].y = color[1];
			_lightingShaderInfo.colors[curLight].z = color[2];

			curLight++;
		}
	}
	_lightingShaderInfo.nLights = curLight;

	uint32_t align = _mainDevice.getPhysicalDevice()->getProps().limits.minUniformBufferOffsetAlignment;
	uint32_t offset1 = sizeof(ViewShaderInfo) + align - (sizeof(ViewShaderInfo) % align);
	_uniformBuffs[idx].setData(&_lightingShaderInfo, offset1, sizeof(_lightingShaderInfo));
	t += 0.01f;
}

VkFramebuffer& VulkanRenderer::_getCurrentFrameBuffer()
{
	return _frameBuffs[_currentSwapchainImgID];
}

