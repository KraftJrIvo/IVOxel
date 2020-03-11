#include "GPURenderer.h"

#include <iostream>

GPURenderer::GPURenderer()
{
	std::vector<uint32_t> queueFamilies = { VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT };
	std::vector<VkPhysicalDeviceType> deviceTypesByPriority = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};

	_vulkan.chooseDevice(queueFamilies, deviceTypesByPriority);
	

	auto mainDevice = _vulkan.getDevice();
	
	if (mainDevice.getDevice() == VK_NULL_HANDLE)
		std::cout << "No device was chosen." << std::endl;
	else
	{
		std::cout << "Device was chosen: " << _vulkan.getPhysDevice().getProps().deviceName << std::endl;

		VkCommandBuffer* commands = new VkCommandBuffer[3];
		mainDevice.getCommand(commands, 1, queueFamilies[0]);
		mainDevice.getCommand(commands + 1, 2, queueFamilies[1]);
		std::cout << "Allocated 1 compute command and 2 graphics command successfully." << std::endl;

		mainDevice.freeCommand(commands, 1, queueFamilies[0]);
		mainDevice.freeCommand(commands + 1, 2, queueFamilies[1]);
		std::cout << "Freed all the commands successfully." << std::endl;
	}
}

void GPURenderer::render(const VoxelMap& map, Camera& cam) const
{
}
