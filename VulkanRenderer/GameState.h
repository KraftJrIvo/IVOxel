#pragma once

#include <list>
#include <chrono>

#include <glm/glm.hpp>

#include <VoxelMap.h>

#include "VulkanDescriptorPool.h"
#include "ShaderData.h"

class Window;

class GameState
{
public:
	void init(Window* window);
	
	void setMap(const VoxelMap& map);
	
	glm::vec4 getRenderArea() const;
	glm::vec2 getRotDelta() const;
	glm::vec3 getTransDelta() const;
	
	void updateRot();
	void updateTrans();

	const std::list<std::shared_ptr<ShaderData>>& getShaderData() { return _shaderData; }
	const glm::vec2& getRot() { return _curRot; }
	const glm::vec3& getTrans() { return _curTrans; }
	VoxelMap& getMap() { return _map; }
	float getTime();
	
	void update(VulkanDescriptorPool& pool, uint32_t frameID);

private:
	Window* _window;
	VoxelMap _map;

	std::list<std::shared_ptr<ShaderData>> _shaderData;

	glm::vec2 _curRot;
	glm::vec3 _curTrans;

	std::chrono::steady_clock::time_point _startTime;
};