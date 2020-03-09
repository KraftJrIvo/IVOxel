#include "VulkanTypes.h"

VkApplicationInfo vkTypes::getAppInfo()
{
	VkApplicationInfo info = {};

    info.sType              =   VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pNext              =   NULL;
    info.pApplicationName   =   "test";
    info.applicationVersion =   VK_MAKE_VERSION(0, 0, 1);
    info.pEngineName        =   "IVOxel";
    info.engineVersion      =   VK_MAKE_VERSION(0, 0, 1);
    info.apiVersion         =   VK_API_VERSION_1_0;

	return info;
}

VkInstanceCreateInfo vkTypes::getInstanceCreateInfo(const VkApplicationInfo& appInfo, std::vector<const char*>& layers, std::vector<const char*>& extensions)
{
	VkInstanceCreateInfo info = {};
    
    info.sType                   =   VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pNext                   =   NULL;
    info.flags                   =   0;
    info.pApplicationInfo        =   &appInfo;
    info.enabledLayerCount       =   0;//layers.size();
    info.ppEnabledLayerNames     = NULL;//layers.data();
    info.enabledExtensionCount   = 0;//extensions.size();
    info.ppEnabledExtensionNames = NULL;//extensions.data();

	return info;
}
