#include "VulkanRenderer.h"
#include "Window.h"
#include "VulkanBuffer.h"

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

	auto mainDevice = _vulkan.getDevice();

	if (mainDevice.getDevice() == VK_NULL_HANDLE)
		std::cout << std::endl << "No device was chosen." << std::endl;
	else
	{
		std::cout << std::endl << "Device was chosen: " << _vulkan.getPhysDevice().getProps().deviceName << std::endl;

		_setSurfaceFormat(mainDevice);
		_initSwapchain();
		_initSwapchainImages();
		_initDepthStencilImage(mainDevice);

		_initRenderPass(mainDevice);

		VkCommandBuffer* commands = new VkCommandBuffer[3];
		uint32_t id1 = mainDevice.getQFIdByType(queueFamilies[0]);
		uint32_t id2 = mainDevice.getQFIdByType(queueFamilies[1]);
		mainDevice.getCommand(commands, 1, id1);
		mainDevice.getCommand(commands + 1, 2, id2);
		std::cout << std::endl << "Allocated 1 compute command and 2 graphics commands successfully." << std::endl << std::endl;

		float arr[3] = { 1.0f, 2.0f, 3.0f };
		VulkanBuffer buf(mainDevice, &arr, sizeof(float), 3);
		buf.allocate();
		std::cout << "Uniform buffer created successfully." << std::endl;

		buf.setData(1, 1);
		std::cout << "Sample data successfully sent to GPU." << std::endl;

		mainDevice.freeCommand(commands, 1, id1);
		mainDevice.freeCommand(commands + 1, 2, id2);
		std::cout << "Freed all the commands successfully." << std::endl;
	}
}

void VulkanRenderer::run()
{
	while (runOnce())
	{
	}
}

bool VulkanRenderer::runOnce()
{
	return window.Update();
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
	if (bufferSz < _surfaceCapabs.minImageCount + 1) bufferSz = _surfaceCapabs.minImageCount + 1;
	if (_surfaceCapabs.maxImageCount > 0 && bufferSz > _surfaceCapabs.maxImageCount) bufferSz = _surfaceCapabs.maxImageCount;

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

void VulkanRenderer::_initDepthStencilImage(const VulkanDevice& device)
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

	VkFormat chosenFormat = VK_FORMAT_UNDEFINED;
	VkFormatProperties fProps;
	for (auto& format : formatsToTry)
	{
		vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice()->getDevice(), format, &fProps);
		if (fProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			chosenFormat = format;
			break;
		}
	}
	
	auto imgInfo = vkTypes::getImageCreateInfo(VK_IMAGE_TYPE_2D, chosenFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
		_surfaceCapabs.currentExtent.width, _surfaceCapabs.currentExtent.height);

	vkCreateImage(device.getDevice(), &imgInfo, nullptr, &_depthStencilImg);

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(device.getDevice(), _depthStencilImg, &memReq);

	uint32_t memId = _getMemoryId(device, memReq, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	auto allocInfo = vkTypes::getMemAllocInfo(memReq.size, memId);
	vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &_depthStencilImgMem);
	vkBindImageMemory(device.getDevice(), _depthStencilImg, _depthStencilImgMem, 0);

	auto rgba = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentMapping mapping = { rgba, rgba, rgba, rgba };
	VkImageSubresourceRange subRng;
	subRng.aspectMask	  = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	subRng.baseMipLevel   = 0;
	subRng.levelCount	  = 1;
	subRng.baseArrayLayer = 0;
	subRng.layerCount	  = 1;

	auto viewInfo = vkTypes::getImageViewCreateInfo(_depthStencilImg, mapping, subRng, chosenFormat, VK_IMAGE_VIEW_TYPE_2D);

	vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &_depthStencilImgView);
}

void VulkanRenderer::_initRenderPass(const VulkanDevice& device)
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

	auto info = vkTypes::getRenderPassCreateInfo(attachments, subpasses);

	vkCreateRenderPass(device.getDevice(), &info, nullptr, &_renderPass);
}

void VulkanRenderer::_setSurfaceFormat(const VulkanDevice& device)
{
	auto physDev = device.getPhysicalDevice()->getDevice();
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDev, _surface, &_surfaceCapabs);
	if (_surfaceCapabs.currentExtent.width < UINT32_MAX)
		window.setSurfaceSize(_surfaceCapabs.currentExtent.width, _surfaceCapabs.currentExtent.height);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, _surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, _surface, &formatCount, formats.data());
	_surfaceFormat = formats[0];
}

uint32_t VulkanRenderer::_getMemoryId(const VulkanDevice& device, const VkMemoryRequirements& memReq, VkMemoryPropertyFlagBits reqFlags)
{
	auto devMemProps = device.getPhysicalDevice()->getMemProps();	
	for (uint32_t i = 0; i < devMemProps.memoryTypeCount; ++i)
		if (memReq.memoryTypeBits & (1 << i))
			if ((devMemProps.memoryTypes[i].propertyFlags & reqFlags) == reqFlags)
				return i;
	return -1;
}

