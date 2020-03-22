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

    info.sType       =  VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.pNext       =  nullptr;
    info.size        =  size;
    info.sharingMode =  VK_SHARING_MODE_EXCLUSIVE;
    info.usage       =  usageFlags;

    return info;
}

VkMemoryAllocateInfo vkTypes::getMemAllocInfo(VkDeviceSize size, uint32_t memTypeId)
{
    VkMemoryAllocateInfo info = {};

    info.sType           =  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.pNext           =  nullptr;
    info.allocationSize  =  size;
    info.memoryTypeIndex =  memTypeId;

    return info;
}

VkWin32SurfaceCreateInfoKHR vkTypes::getSurfaceCreateInfo(const HINSTANCE& hinstance, const HWND& hwnd)
{
    VkWin32SurfaceCreateInfoKHR info = {};

    info.sType     =    VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    info.pNext     =    nullptr;
    info.hinstance =    hinstance;
    info.hwnd      =    hwnd;

    return info;
}

VkSwapchainCreateInfoKHR vkTypes::getSwapchainCreateInfo(const VkSurfaceKHR& surface, const VkSurfaceFormatKHR& format, const VkPresentModeKHR& pm,
    uint32_t bufferSz, uint32_t width, uint32_t height)
{
    VkSwapchainCreateInfoKHR info = {};
    info.sType                 =    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    //info.pNext                 =    nullptr;
    info.surface               =    surface;
    info.minImageCount         =    bufferSz;
    info.imageFormat           =    format.format;
    info.imageColorSpace       =    format.colorSpace;
    info.imageExtent.width     =    width;
    info.imageExtent.height    =    height;
    info.imageArrayLayers      =    1;
    info.imageUsage            =    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.imageSharingMode      =    VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount =    0;
    info.pQueueFamilyIndices   =    nullptr;
    info.preTransform          =    VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.compositeAlpha        =    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode           =    pm;
    info.clipped               =    VK_TRUE;
    info.oldSwapchain          =    VK_NULL_HANDLE;

    return info;
}
