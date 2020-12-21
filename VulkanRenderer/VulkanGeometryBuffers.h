#pragma once

#include "VulkanDevice.h"
#include "VulkanBuffer.h"

class VulkanGeometryBuffers
{
public:
	void init(const VulkanDevice& device);
	void bindCmdBuffer(const VulkanDevice& device, const VkCommandBuffer& buf);

	uint32_t getIndicesCount();
	uint32_t getVerticesCount();

private:
	VulkanBuffer _vertexBuff;
	VulkanBuffer _indexBuff;
};
