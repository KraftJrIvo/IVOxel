#include "VulkanGeometryBuffers.h"
#include "Vertex.h"

void VulkanGeometryBuffers::init(const VulkanDevice& device)
{
	// creating vertex buffer 
	std::vector<Vertex> vertices = {
		{{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
		{{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
		{{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
		{{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
	};
	_vertexBuff.initStaging(device, vertices.data(), sizeof(Vertex), vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	// creating index buffer 
	std::vector<uint16_t> indices = { 0,1,2,2,3,0 };
	_indexBuff.initStaging(device, indices.data(), sizeof(uint16_t), indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

}

void VulkanGeometryBuffers::bindCmdBuffer(const VulkanDevice& device, const VkCommandBuffer& buf)
{
	std::vector<VkBuffer> vertexBuffers = { _vertexBuff.getBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(buf, 0, 1, vertexBuffers.data(), offsets);

	vkCmdBindIndexBuffer(buf, _indexBuff.getBuffer(), 0, VK_INDEX_TYPE_UINT16);
}

uint32_t VulkanGeometryBuffers::getIndicesCount()
{
	return _indexBuff.getElemsCount();
}

uint32_t VulkanGeometryBuffers::getVerticesCount()
{
	return _vertexBuff.getElemsCount();
}
