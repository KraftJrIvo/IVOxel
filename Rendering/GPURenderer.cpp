#include "GPURenderer.h"

GPURenderer::GPURenderer()
{
	// instanse creation
	VkApplicationInfo app_info = vkTypes::getAppInfo();	
	VkInstanceCreateInfo instance_info = vkTypes::getInstanceCreateInfo(app_info, _layers, _extensions);

	VulkanInstance _vulkan(instance_info);

	// getting the device
	_vulkan.choosePhysicalDevice();
}

void GPURenderer::render(const VoxelMap& map, Camera& cam) const
{
}
