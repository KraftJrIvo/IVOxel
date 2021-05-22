#pragma once

#include <list>
#include <memory>

#include <GameDataContainer.h>
#include <GameData.h>

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"

struct ShaderData;

class VulkanDescriptorPool : public GameDataContainer
{
public:
	void initLayouts(const VulkanDevice& device, const std::list<std::shared_ptr<GameData>>& gameData);
	void init(const VulkanDevice& device, const VulkanSwapchain& swapchain);
	void destroy(const VulkanDevice& device);

	const std::vector<VkDescriptorSetLayout>& getLayouts();
	const std::vector<VkDescriptorSet>& getSets(uint32_t frameID);

	void setData(uint32_t dataID, void* ptr, uint32_t frameID) override;

private:
	VkDescriptorPool _pool;
	std::vector<VkDescriptorSetLayout> _setLayouts;
	std::vector<std::vector<VkDescriptorSet>> _setsByType;
	std::vector<std::vector<VkDescriptorSet>> _setsByFrame;
	std::vector<VulkanBuffer> _uniformBuffs;
	std::vector<uint32_t> _setSizes;
	std::vector<uint32_t> _offsets;
};