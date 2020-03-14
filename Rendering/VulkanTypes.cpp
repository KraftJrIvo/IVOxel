#include "VulkanTypes.h"

VkApplicationInfo vkTypes::getAppInfo()
{
	VkApplicationInfo info = {};

    info.sType              =   VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pNext              =   nullptr;
    info.pApplicationName   =   "test";
    info.applicationVersion =   VK_MAKE_VERSION(0, 0, 1);
    info.pEngineName        =   "IVOxel";
    info.engineVersion      =   VK_MAKE_VERSION(0, 0, 1);
    info.apiVersion         =   VK_API_VERSION_1_0;

	return info;
}

VkInstanceCreateInfo vkTypes::getInstanceCreateInfo(const VkApplicationInfo& appInfo, const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	VkInstanceCreateInfo info = {};
    
    info.sType                   =   VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pNext                   =   nullptr;
    info.flags                   =   0;
    info.pApplicationInfo        =   &appInfo;
    info.enabledLayerCount       =   layers.size();
    info.ppEnabledLayerNames     =   layers.data();
    info.enabledExtensionCount   =   extensions.size();
    info.ppEnabledExtensionNames =   extensions.data();

	return info;
}

VkDeviceQueueCreateInfo vkTypes::getQFCreateInfo(uint32_t queueCount, uint32_t familyId, float* priority)
{
    VkDeviceQueueCreateInfo info = {};
	
    info.sType            =     VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    info.pNext            =     nullptr;
    info.flags            =     0;
    info.queueCount       =     queueCount;
    info.queueFamilyIndex =     familyId;
    info.pQueuePriorities =     priority;

    return info;
}

VkDeviceCreateInfo vkTypes::getDeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueInfos, const VkPhysicalDeviceFeatures* feats,
    const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
    VkDeviceCreateInfo info = {};

    info.sType                   =  VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.pNext                   =  nullptr;
    info.flags                   =  0;
    info.queueCreateInfoCount    =  queueInfos.size();
    info.pQueueCreateInfos       =  queueInfos.data();
    info.enabledLayerCount       =  layers.size();
    info.ppEnabledLayerNames     =  layers.data();
    info.enabledExtensionCount   =  extensions.size();
    info.ppEnabledExtensionNames =  extensions.data();
    info.pEnabledFeatures        =  feats;

    return info;
}

VkCommandPoolCreateInfo vkTypes::getCPCreateInfo(uint32_t familyId, VkCommandPoolCreateFlags flags)
{
    VkCommandPoolCreateInfo info = {};
    
    info.sType            =     VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext            =     nullptr;
    info.flags            =     flags;
    info.queueFamilyIndex =     familyId;
    
    return info;
}

VkCommandBufferAllocateInfo vkTypes::getCBAllocateInfo(const VkCommandPool& pool, uint32_t count, VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo info = {};

    info.sType              =   VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext              =   nullptr;
    info.level              =   level;
    info.commandPool        =   pool;
    info.commandBufferCount =   count;

    return info;
}

VkBufferCreateInfo vkTypes::getBufCreateInfo(VkDeviceSize size, VkBufferUsageFlags usageFlags)
{
    VkBufferCreateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.pNext = nullptr;
    info.size = size;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.usage = usageFlags;

    return info;
}

VkMemoryAllocateInfo vkTypes::getMemAllocInfo(VkDeviceSize size, uint32_t memTypeId)
{
    VkMemoryAllocateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.allocationSize = size;
    info.memoryTypeIndex = memTypeId;

    return info;
}
