#pragma once

#include <stdint.h>

class VulkanDescriptorPool;
class GameState;

struct ShaderData
{
	virtual void update(VulkanDescriptorPool& pool, uint32_t frameID, uint32_t dataID, GameState& game) = 0;

	uint32_t size;
};
