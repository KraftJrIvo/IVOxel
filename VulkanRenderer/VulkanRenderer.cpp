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
		auto info = vkTypes::getImageViewCreateInfo(_swapchainImgs[i], _surfaceFormat, mapping, subRng, VK_IMAGE_VIEW_TYPE_2D);
		vkCreateImageView(_vulkan.getDevice().getDevice(), &info, nullptr, &_swapchainImgViews[i]);
	}
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

