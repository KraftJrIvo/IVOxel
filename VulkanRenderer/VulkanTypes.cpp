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

VkCommandBufferBeginInfo vkTypes::getCBBeginInfo(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    //info.pInheritanceInfo;

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

VkImageCreateInfo vkTypes::getImageCreateInfo(VkImageType type, VkFormat format, VkImageUsageFlagBits usage, uint32_t w, uint32_t h)
{
    VkImageCreateInfo info = {};

    info.sType                 =    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext                 =    nullptr;
    info.flags                 =    0;
    info.imageType             =    type;
    info.format                =    format;
    info.extent                =    {w, h, 1};
    info.mipLevels             =    1;
    info.arrayLayers           =    1;
    info.samples               =    VK_SAMPLE_COUNT_1_BIT;
    info.tiling                =    VK_IMAGE_TILING_OPTIMAL;
    info.usage                 =    usage;
    info.sharingMode           =    VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount =    VK_QUEUE_FAMILY_IGNORED;
    info.pQueueFamilyIndices   =    nullptr;
    info.initialLayout         =    VK_IMAGE_LAYOUT_UNDEFINED;

    return info;
}

VkImageViewCreateInfo vkTypes::getImageViewCreateInfo(const VkImage& img, const VkComponentMapping& mapping, const VkImageSubresourceRange& subRng, VkFormat format, VkImageViewType type)
{
    VkImageViewCreateInfo info = {};
    
    info.sType            =     VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext            =     nullptr;
    info.flags            =     0;
    info.image            =     img;
    info.viewType         =     type;
    info.format           =     format;
    info.components       =     mapping;
    info.subresourceRange =     subRng;

    return info;
}

VkRenderPassCreateInfo vkTypes::getRPCreateInfo(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subPasses)
{
    VkRenderPassCreateInfo info = {};

    info.sType           =  VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.pNext           =  nullptr;
    info.flags           =  0;
    info.attachmentCount =  attachments.size();
    info.pAttachments    =  attachments.data();
    info.subpassCount    =  subPasses.size();
    info.pSubpasses      =  subPasses.data();
    //info.dependencyCount;
    //info.pDependencies;

    return info;
}

VkRenderPassBeginInfo vkTypes::getRPBeginInfo(const VkRenderPass& renderPass, const VkFramebuffer& frameBuff, const VkRect2D& area, const std::vector<VkClearValue>& clearVals)
{
    VkRenderPassBeginInfo info = {};

    info.sType           =   VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.pNext           =   nullptr;
    info.renderPass      =   renderPass;
    info.framebuffer     =   frameBuff;
    info.renderArea      =   area;
    info.clearValueCount =   clearVals.size();
    info.pClearValues    =   clearVals.data();
    
    return info;
}

VkFramebufferCreateInfo vkTypes::getFramebufferCreateInfo(const VkRenderPass& renderPass, const std::vector<VkImageView>& attachments, uint32_t w, uint32_t h, uint32_t layers)
{
    VkFramebufferCreateInfo info = {};

    info.sType           =  VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.pNext           =  nullptr;
    info.flags           =  0;
    info.renderPass      =  renderPass;
    info.attachmentCount =  attachments.size();
    info.pAttachments    =  attachments.data();
    info.width           =  w;
    info.height          =  h;
    info.layers          =  layers;

    return info;
}

VkFenceCreateInfo vkTypes::getFenceCreateInfo()
{
    VkFenceCreateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    return info;
}

VkPresentInfoKHR vkTypes::getPresentInfo(const std::vector<VkSemaphore>& semaphores, const std::vector<VkSwapchainKHR>& swapchains, 
    const std::vector<uint32_t>& imageIds, std::vector<VkResult>& results)
{
    VkPresentInfoKHR info = {};

    info.sType              =   VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.pNext              =   nullptr;
    info.waitSemaphoreCount =   semaphores.size();
    info.pWaitSemaphores    =   semaphores.data();
    info.swapchainCount     =   swapchains.size();
    info.pSwapchains        =   swapchains.data();
    info.pImageIndices      =   imageIds.data();
    info.pResults           =   results.data();

    return info;
}

VkSubmitInfo vkTypes::getSubmitInfo(const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& signalSemaphores, const std::vector<VkCommandBuffer>& cmdBuffs, uint32_t* dstStageMaskFlags)
{
    VkSubmitInfo info = {};

    info.sType                =     VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext                =     nullptr;
    info.waitSemaphoreCount   =     waitSemaphores.size();
    info.pWaitSemaphores      =     waitSemaphores.data();
    info.pWaitDstStageMask    =     dstStageMaskFlags;
    info.commandBufferCount   =     cmdBuffs.size();
    info.pCommandBuffers      =     cmdBuffs.data();
    info.signalSemaphoreCount =     signalSemaphores.size();
    info.pSignalSemaphores    =     signalSemaphores.data();

    return info;
}

VkSemaphoreCreateInfo vkTypes::getSemaphoreCreateInfo()
{
    VkSemaphoreCreateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;;
    info.pNext = nullptr;
    info.flags = 0;

    return info;
}

VkDescriptorSetLayoutCreateInfo vkTypes::getDSLCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.pNext        = nullptr;
    info.flags        = 0;
    info.bindingCount = bindings.size();
    info.pBindings    = bindings.data();

    return info;
}

VkDescriptorPoolCreateInfo vkTypes::getDescriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& descPoolSizes, uint32_t maxSets)
{
    VkDescriptorPoolCreateInfo info = {};
    info.sType         =    VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount =    descPoolSizes.size();
    info.pPoolSizes    =    descPoolSizes.data();
    info.maxSets       =    maxSets;

    return info;
}

VkDescriptorSetAllocateInfo vkTypes::getDescriptorSetAllocateInfo(const VkDescriptorPool& pool, const std::vector<VkDescriptorSetLayout>& layouts, uint32_t setCount)
{
    VkDescriptorSetAllocateInfo info = {};

    info.sType              =  VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.pNext              =  nullptr;
    info.descriptorPool     =  pool;
    info.descriptorSetCount =  setCount;
    info.pSetLayouts        =  layouts.data();

    return info;
}

VkPipelineVertexInputStateCreateInfo vkTypes::getPipelineVertexISCreateInfo(const std::vector<VkVertexInputBindingDescription>& vibDescrs, const std::vector<VkVertexInputAttributeDescription>& vaDescrs)
{
    VkPipelineVertexInputStateCreateInfo info = {};

    info.sType                           =  VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext                           =  nullptr;
    info.flags                           =  0;
    info.vertexBindingDescriptionCount   =  vibDescrs.size();
    info.pVertexBindingDescriptions      =  vibDescrs.data();
    info.vertexAttributeDescriptionCount =  vaDescrs.size();
    info.pVertexAttributeDescriptions    =  vaDescrs.data();

    return info;
}

VkPipelineInputAssemblyStateCreateInfo vkTypes::getPipelineInputAssemblyISCreateInfo(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnabled)
{
    VkPipelineInputAssemblyStateCreateInfo info = {};

    info.sType                  =   VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext                  =   nullptr;
    info.flags                  =   0;
    info.topology               =   topology;
    info.primitiveRestartEnable =   primitiveRestartEnabled;

    return info;
}

VkPipelineViewportStateCreateInfo vkTypes::getPipelineViewportSCreateInfo(const std::vector<VkViewport>& viewports, const std::vector<VkRect2D>& scissors)
{
    VkPipelineViewportStateCreateInfo info = {};

    info.sType         =    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info.pNext         =    nullptr;
    info.flags         =    0;
    info.viewportCount =    viewports.size();
    info.pViewports    =    viewports.data();
    info.scissorCount  =    scissors.size();
    info.pScissors     =    scissors.data();

    return info;
}

VkPipelineRasterizationStateCreateInfo vkTypes::getPipelineRasterizationSCreateInfo(VkPolygonMode polyMode, VkCullModeFlags cullMode, uint32_t lineWidth, VkFrontFace frontFace)
{
    VkPipelineRasterizationStateCreateInfo info = {};

    info.sType                   =  VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext                   =  nullptr;
    info.flags                   =  0;
    info.polygonMode             =  polyMode;
    info.lineWidth               =  lineWidth;
    info.cullMode                =  cullMode;
    info.frontFace               =  frontFace;
    info.depthBiasEnable         =  VK_FALSE;
    info.depthBiasConstantFactor =  0.0f;
    info.depthBiasClamp          =  0.0f;
    info.depthBiasSlopeFactor    =  0.0f;

    return info;
}

VkPipelineMultisampleStateCreateInfo vkTypes::getPipelineMultisampleSCreateInfo(VkBool32 shading, VkSampleCountFlagBits sampleFlags, float minSampleShading, const VkSampleMask* pMask, VkBool32 alphaToCoverageEnable, VkBool32 alphaToOneEnable)
{
    VkPipelineMultisampleStateCreateInfo info = {};

    info.sType                 =    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext                 =    nullptr;
    info.flags                 =    0;
    info.sampleShadingEnable   =    shading;
    info.rasterizationSamples  =    sampleFlags;
    info.minSampleShading      =    minSampleShading;
    info.pSampleMask           =    pMask;
    info.alphaToCoverageEnable =    alphaToCoverageEnable;
    info.alphaToOneEnable      =    alphaToOneEnable;

    return info;
}

VkPipelineColorBlendStateCreateInfo vkTypes::getPipelineColorBlendSCreateInfo(const std::vector<VkPipelineColorBlendAttachmentState>& attachments, VkBool32 logicOpEnable, VkLogicOp logicOp, std::vector<float> blendConsts)
{
    VkPipelineColorBlendStateCreateInfo info = {};

    info.sType           =    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.pNext           =    nullptr;
    info.flags           =    0;
    info.logicOpEnable   =    logicOpEnable;
    info.logicOp         =    logicOp;
    info.attachmentCount =    attachments.size();
    info.pAttachments    =    attachments.data();
    
    std::memcpy(info.blendConstants, blendConsts.data(), 4 * sizeof(float));

    return info;
}

VkPipelineDepthStencilStateCreateInfo vkTypes::getPipelineDepthStencilSCreateInfo(VkBool32 depthTest, VkBool32 depthWrite, VkCompareOp compareOp, VkBool32 depthBoundsTest, VkBool32 stencilTest, VkStencilOpState front, VkStencilOpState back, float minBound, float maxBound)
{
    VkPipelineDepthStencilStateCreateInfo info = {};

    info.sType                 =    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext                 =    nullptr;
    info.flags                 =    0;
    info.depthTestEnable       =    depthTest;
    info.depthWriteEnable      =    depthWrite;
    info.depthCompareOp        =    compareOp;
    info.depthBoundsTestEnable =    depthBoundsTest;
    info.stencilTestEnable     =    stencilTest;
    info.front                 =    front;
    info.back                  =    back;
    info.minDepthBounds        =    minBound;
    info.maxDepthBounds        =    maxBound;

    return info;
}

VkPipelineLayoutCreateInfo vkTypes::getPipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout>& layouts, const std::vector<VkPushConstantRange>& constantRanges)
{
    VkPipelineLayoutCreateInfo info = {};

    info.sType                  =   VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext                  =   nullptr;
    info.flags                  =   0;
    info.setLayoutCount         =   layouts.size();
    info.pSetLayouts            =   layouts.data();
    info.pushConstantRangeCount =   constantRanges.size();
    info.pPushConstantRanges    =   constantRanges.data();

    return info;
}

VkGraphicsPipelineCreateInfo vkTypes::getGraphicsPipelineCreateInfo(const VkPipelineLayout& layout, const VkRenderPass& renderPass, uint32_t subpass, const std::vector<VkPipelineShaderStageCreateInfo>& stages, const VkPipelineVertexInputStateCreateInfo& vertexInputSCI, const VkPipelineInputAssemblyStateCreateInfo& inputAssemblySCI, const VkPipelineViewportStateCreateInfo& viewportSCI, const VkPipelineRasterizationStateCreateInfo& rasterizationSCI, const VkPipelineMultisampleStateCreateInfo& multisampleSCI, const VkPipelineColorBlendStateCreateInfo& colorBlendSCI, const VkPipelineDepthStencilStateCreateInfo* depthStencilSCI, const VkPipelineDynamicStateCreateInfo* dynamicSCI, VkPipeline basePipeline, uint32_t basePipelineId)
{
    VkGraphicsPipelineCreateInfo info = {};

    info.sType               =  VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.pNext               =  nullptr;
    info.flags               =  0;
    info.stageCount          =  stages.size();
    info.pStages             =  stages.data();
    info.pVertexInputState   =  &vertexInputSCI;
    info.pInputAssemblyState =  &inputAssemblySCI;
    info.pViewportState      =  &viewportSCI;
    info.pRasterizationState =  &rasterizationSCI;
    info.pMultisampleState   =  &multisampleSCI;
    info.pColorBlendState    =  &colorBlendSCI;
    info.pDepthStencilState  =  depthStencilSCI;
    info.pDynamicState       =  dynamicSCI;
    info.layout              =  layout;
    info.renderPass          =  renderPass;
    info.subpass             =  subpass;
    info.basePipelineHandle  =  basePipeline; 
    info.basePipelineIndex   =  basePipelineId;

    return info;
}
