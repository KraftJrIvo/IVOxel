#include "GPURenderer.h"

#include <iostream>

GPURenderer::GPURenderer()
{
	std::vector<uint32_t> queueFamilies = { VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT };
	std::vector<VkPhysicalDeviceType> deviceTypesByPriority = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};

	_vulkan.chooseDevice(queueFamilies, deviceTypesByPriority);

	auto mainDevice = _vulkan.getDevice();
	
	if (mainDevice.getDevice() == VK_NULL_HANDLE)
		std::cout << "No device was chosen." << std::endl;
	else
	{
		std::cout << "Device was chosen: " << _vulkan.getPhysDevice().getProps().deviceName << std::endl;

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
