#include "VulkanDescriptorPool.h"

void VulkanDescriptorPool::initLayouts(const VulkanDevice& device, const std::list<std::shared_ptr<GameData>>& gameData)
{
	for (auto& data : gameData)
		_setSizes.push_back(data->size);
	_setLayouts.resize(_setSizes.size());

	for (auto& layout : _setLayouts)
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding };
		VkDescriptorSetLayoutCreateInfo info = vkTypes::getDSLCreateInfo(bindings);

		vkCreateDescriptorSetLayout(device.get(), &info, nullptr, &layout);
	}

	_setsByType.resize(_setSizes.size());
}

void VulkanDescriptorPool::init(const VulkanDevice& device, const VulkanSwapchain& swapchain)
{
	_setsByFrame.resize(swapchain.getImgCount());

	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 3 * static_cast<uint32_t>(swapchain.getImgCount());

	std::vector<VkDescriptorPoolSize> sizes = { poolSize };
	auto info = vkTypes::getDescriptorPoolCreateInfo(sizes, 3 * swapchain.getImgCount());

	vkCreateDescriptorPool(device.get(), &info, nullptr, &_pool);

	uint32_t align = device.getPhysicalDevice()->getProps().limits.minUniformBufferOffsetAlignment;
	_offsets = {0};
	for (int i = 1; i <= _setLayouts.size(); ++i)
		_offsets.push_back(_offsets[i - 1] + align * (uint32_t)ceil((float)_setSizes[i - 1] / (float)align));

	uint32_t sumSize = _offsets[_offsets.size() - 1];
	_uniformBuffs.resize(swapchain.getImgCount());
	for (uint32_t i = 0; i < swapchain.getImgCount(); ++i)
		_uniformBuffs[i].initHost(device, sumSize, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	for (int i = 0; i < _setLayouts.size(); ++i)
	{
		std::vector<VkDescriptorSetLayout> layouts(swapchain.getImgCount(), _setLayouts[i]);
		auto allocInfo = vkTypes::getDescriptorSetAllocateInfo(_pool, layouts, swapchain.getImgCount());

		_setsByType[i].resize(swapchain.getImgCount());
		vkAllocateDescriptorSets(device.get(), &allocInfo, _setsByType[i].data());

		for (int j = 0; j < swapchain.getImgCount(); j++) {
			_setsByFrame[j].resize(_setSizes.size());
			_setsByFrame[j][i] = _setsByType[i][j];

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = _uniformBuffs[j].getBuffer();
			bufferInfo.offset = _offsets[i];
			bufferInfo.range = _setSizes[i];

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = _setsByType[i][j];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device.get(), 1, &descriptorWrite, 0, nullptr);
		}
	}
}

void VulkanDescriptorPool::destroy(const VulkanDevice& device)
{
	vkDestroyDescriptorPool(device.get(), _pool, nullptr);
	for (auto& layout : _setLayouts)
		vkDestroyDescriptorSetLayout(device.get(), layout, nullptr);
}
const std::vector<VkDescriptorSetLayout>& VulkanDescriptorPool::getLayouts()
{
	return _setLayouts;
}

const std::vector<VkDescriptorSet>& VulkanDescriptorPool::getSets(uint32_t frameID)
{
	return _setsByFrame[frameID];
}

void VulkanDescriptorPool::setData(uint32_t dataID, void* ptr, uint32_t frameID)
{
	_uniformBuffs[frameID].setData(ptr, _offsets[dataID], _setSizes[dataID]);
}

