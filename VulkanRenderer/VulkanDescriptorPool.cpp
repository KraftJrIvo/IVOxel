#include "VulkanDescriptorPool.h"
#include <iostream>

void VulkanDescriptorPool::initLayouts(const VulkanDevice& device, const std::list<std::shared_ptr<GameData>>& gameData)
{
	for (uint32_t i = 0; i < gameData.size(); ++i)
	{
		auto it = gameData.begin();
		std::advance(it, i);
		auto data = *it;
		if (data->size < 64000)
		{
			_typesAndIds.push_back({ UNIFORM, _uniformSetSizes.size() });
			_uniformSetSizes.push_back(data->size);
		}
		else
		{
			_typesAndIds.push_back({ STORAGE, _storageSetSizes.size() });
			_storageSetSizes.push_back(data->size);
		}
	}
	_uniformSetLayouts.resize(_uniformSetSizes.size());
	_storageSetLayouts.resize(_storageSetSizes.size());
	_uniformSetsByType.resize(_uniformSetSizes.size());
	_storageSetsByType.resize(_storageSetSizes.size());

	for (auto& layout : _uniformSetLayouts)
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		
		std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding };
		VkDescriptorSetLayoutCreateInfo info = vkTypes::getDSLCreateInfo(bindings);

		vkCreateDescriptorSetLayout(device.get(), &info, nullptr, &layout);
		_allSetLayouts.push_back(layout);
	}
	
	for (auto& layout : _storageSetLayouts)
	{
		VkDescriptorSetLayoutBinding sboLayoutBinding = {};
		sboLayoutBinding.binding = 0;
		sboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		sboLayoutBinding.descriptorCount = 1;
		sboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		std::vector<VkDescriptorSetLayoutBinding> bindings = { sboLayoutBinding };
		VkDescriptorSetLayoutCreateInfo info = vkTypes::getDSLCreateInfo(bindings);
		vkCreateDescriptorSetLayout(device.get(), &info, nullptr, &layout);
		_allSetLayouts.push_back(layout);
	}
}

void VulkanDescriptorPool::init(const VulkanDevice& device, const VulkanSwapchain& swapchain)
{
	_setsByFrame.resize(swapchain.getImgCount());

	VkDescriptorPoolSize poolSizeUniform = {};
	poolSizeUniform.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizeUniform.descriptorCount = _uniformSetLayouts.size() * static_cast<uint32_t>(swapchain.getImgCount());
	
	VkDescriptorPoolSize poolSizeStorage = {};
	poolSizeStorage.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizeStorage.descriptorCount = _storageSetLayouts.size() * static_cast<uint32_t>(swapchain.getImgCount());

	std::vector<VkDescriptorPoolSize> sizes = { poolSizeUniform, poolSizeStorage };
	auto info = vkTypes::getDescriptorPoolCreateInfo(sizes, _uniformSetLayouts.size() * swapchain.getImgCount() + 
		_storageSetLayouts.size() * swapchain.getImgCount());

	vkCreateDescriptorPool(device.get(), &info, nullptr, &_pool);

	uint32_t align = device.getPhysicalDevice()->getProps().limits.minUniformBufferOffsetAlignment;
	_uniformOffsets = {0};
	for (int i = 1; i <= _uniformSetLayouts.size(); ++i)
		_uniformOffsets.push_back(_uniformOffsets[i - 1] + align * (uint32_t)ceil((float)_uniformSetSizes[i - 1] / (float)align));
	_storageOffsets = { 0 };
	for (int i = 1; i <= _storageSetLayouts.size(); ++i)
		_storageOffsets.push_back(_storageOffsets[i - 1] + align * (uint32_t)ceil((float)_storageSetSizes[i - 1] / (float)align));

	uint32_t sumSize = _uniformOffsets[_uniformOffsets.size() - 1];
	_uniformBuffs.resize(swapchain.getImgCount());
	for (uint32_t i = 0; i < swapchain.getImgCount(); ++i)
		_uniformBuffs[i].initHost(device, sumSize, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	sumSize = _storageOffsets[_storageOffsets.size() - 1];
	_storageBuffs.resize(swapchain.getImgCount());
	for (uint32_t i = 0; i < swapchain.getImgCount(); ++i)
		_storageBuffs[i].initHost(device, sumSize, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	for (int i = 0; i < _uniformSetLayouts.size(); ++i)
	{
		std::vector<VkDescriptorSetLayout> layouts(swapchain.getImgCount(), _uniformSetLayouts[i]);
		auto allocInfo = vkTypes::getDescriptorSetAllocateInfo(_pool, layouts, swapchain.getImgCount());

		_uniformSetsByType[i].resize(swapchain.getImgCount());
		vkAllocateDescriptorSets(device.get(), &allocInfo, _uniformSetsByType[i].data());
		for (int j = 0; j < swapchain.getImgCount(); j++) {
			_setsByFrame[j].resize(_uniformSetSizes.size());
			_setsByFrame[j][i] = _uniformSetsByType[i][j];

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = _uniformBuffs[j].getBuffer();
			bufferInfo.offset = _uniformOffsets[i];
			bufferInfo.range = _uniformSetSizes[i];

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = _uniformSetsByType[i][j];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device.get(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	for (int i = 0; i < _storageSetLayouts.size(); ++i)
	{
		std::vector<VkDescriptorSetLayout> layouts(1, _storageSetLayouts[i]);
		auto allocInfo = vkTypes::getDescriptorSetAllocateInfo(_pool, layouts, 1);

		_storageSetsByType[i].resize(1);
		vkAllocateDescriptorSets(device.get(), &allocInfo, _storageSetsByType[i].data());
		for (int j = 0; j < swapchain.getImgCount(); j++) {
			_setsByFrame[j].push_back(_storageSetsByType[i][0]);

		}
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = _storageBuffs[0].getBuffer();
			bufferInfo.offset = _storageOffsets[i];
			bufferInfo.range = _storageSetSizes[i];

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = _storageSetsByType[i][0];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device.get(), 1, &descriptorWrite, 0, nullptr);
		
	}
}

void VulkanDescriptorPool::destroy(const VulkanDevice& device)
{
	vkDestroyDescriptorPool(device.get(), _pool, nullptr);
	for (auto& layout : _uniformSetLayouts)
		vkDestroyDescriptorSetLayout(device.get(), layout, nullptr);
}

const std::vector<VkDescriptorSetLayout>& VulkanDescriptorPool::getLayouts()
{
	return _allSetLayouts;
}

const std::vector<VkDescriptorSet>& VulkanDescriptorPool::getSets(uint32_t frameID)
{
	return _setsByFrame[frameID];
}

void VulkanDescriptorPool::setData(uint32_t dataID, void* ptr, uint32_t frameID)
{
	bool uniform = _typesAndIds[dataID].first == UNIFORM;

	uint32_t offset = uniform ?
		_uniformOffsets[_typesAndIds[dataID].second] : 
		_storageOffsets[_typesAndIds[dataID].second];

	uint32_t size = uniform ?
		_uniformSetSizes[_typesAndIds[dataID].second] :
		_storageSetSizes[_typesAndIds[dataID].second];

	if (uniform)
		_uniformBuffs[frameID].setData(ptr, offset, size);
	else 
		_storageBuffs[0].setData(ptr, offset, size);
}

