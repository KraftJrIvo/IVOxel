#include "GPURenderer.h"

#include <iostream>

GPURenderer::GPURenderer() :
	_vulkan({ "VK_LAYER_KHRONOS_validation" }, { "VK_EXT_debug_report" })
{
	_outputSupportedInstanceLayers();
	_outputSupportedInstanceExtensions("VK_LAYER_KHRONOS_validation");

	std::vector<uint32_t> queueFamilies = { VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT };
	std::vector<VkPhysicalDeviceType> deviceTypesByPriority = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};

	_vulkan.chooseDevice(queueFamilies, deviceTypesByPriority);

	auto mainDevice = _vulkan.getDevice();
	
	if (mainDevice.getDevice() == VK_NULL_HANDLE)
		std::cout << std::endl << "No device was chosen." << std::endl;
	else
	{
		std::cout << std::endl << "Device was chosen: " << _vulkan.getPhysDevice().getProps().deviceName << std::endl;

		_outputSupportedDeviceLayers();
		_outputSupportedDeviceExtensions();

		VkCommandBuffer* commands = new VkCommandBuffer[3];
		uint32_t id1 = mainDevice.getQFIdByType(queueFamilies[0]);
		uint32_t id2 = mainDevice.getQFIdByType(queueFamilies[1]);
		mainDevice.getCommand(commands, 1, id1);
		mainDevice.getCommand(commands + 1, 2, id2);
		std::cout << std::endl << "Allocated 1 compute command and 2 graphics command successfully." << std::endl << std::endl;

		mainDevice.freeCommand(commands, 1, id1);
		mainDevice.freeCommand(commands + 1, 2, id2);
		std::cout << "Freed all the commands successfully." << std::endl;
	}
}

void GPURenderer::render(const VoxelMap& map, Camera& cam) const
{
}

void GPURenderer::_outputSupportedDeviceExtensions()
{
	auto& physDevice = _vulkan.getPhysDevice();
	uint32_t _extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physDevice.getDevice(), nullptr, &_extensionCount, NULL);
	std::vector<VkExtensionProperties> extProps(_extensionCount);
	vkEnumerateDeviceExtensionProperties(physDevice.getDevice(), nullptr, &_extensionCount, extProps.data());
	std::cout << "Supported extensions: " << std::endl;
	for (uint32_t i = 0; i < _extensionCount; i++) {
		std::cout << "\"" << extProps[i].extensionName << "\"" << std::endl;
	}
}

void GPURenderer::_outputSupportedDeviceLayers()
{
	auto& physDevice = _vulkan.getPhysDevice();
	uint32_t _layerCount = 0;
	vkEnumerateDeviceLayerProperties(physDevice.getDevice(), &_layerCount, NULL);
	std::vector<VkLayerProperties> lProps(_layerCount);
	vkEnumerateDeviceLayerProperties(physDevice.getDevice(), &_layerCount, lProps.data());
	std::cout << "Supported layers: " << std::endl;
	for (uint32_t i = 0; i < _layerCount; i++) {
		std::cout << "\"" << lProps[i].layerName << "\"" << std::endl;
	}
}

void GPURenderer::_outputSupportedInstanceExtensions(const char* layerName)
{
	auto& physDevice = _vulkan.getPhysDevice();
	uint32_t _extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(layerName, &_extensionCount, NULL);
	std::vector<VkExtensionProperties> extProps(_extensionCount);
	vkEnumerateInstanceExtensionProperties(layerName, &_extensionCount, extProps.data());
	std::cout << "Supported instance extensions: " << std::endl;
	for (uint32_t i = 0; i < _extensionCount; i++) {
		std::cout << "\"" << extProps[i].extensionName << "\"" << std::endl;
	}
}

void GPURenderer::_outputSupportedInstanceLayers()
{
	auto& physDevice = _vulkan.getPhysDevice();
	uint32_t _layerCount = 0;
	vkEnumerateInstanceLayerProperties(&_layerCount, NULL);
	std::vector<VkLayerProperties> lProps(_layerCount);
	vkEnumerateInstanceLayerProperties(&_layerCount, lProps.data());
	std::cout << "Supported instance layers: " << std::endl;
	for (uint32_t i = 0; i < _layerCount; i++)
		std::cout << "\"" << lProps[i].layerName << "\"" << std::endl;
}

