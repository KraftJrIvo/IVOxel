#include "Window.h"

#include "GameState.h"

#include "ShaderDataCamera.h"
#include "ShaderDataLight.h"
#include "ShaderDataMap.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

void GameState::init(Window* window)
{ 
	_window = window;
	_curRot = glm::vec2(0.0f, -90.0f);
	_curTrans = glm::vec3(1.5f, 0.5f, 0.5f);
	_startTime = std::chrono::high_resolution_clock::now();

	_shaderData = { std::make_shared<ShaderDataCamera>(), std::make_shared<ShaderDataLight>(), std::make_shared<ShaderDataMap>() };
}

void GameState::setMap(const VoxelMap& map)
{
	_map = map;
}

glm::vec4 GameState::getRenderArea() const
{
	auto ra = _window->getRenderArea();
	return glm::vec4(ra.offset.x, ra.offset.y, ra.extent.width, ra.extent.height);
}

glm::vec2 GameState::getRotDelta() const
{
	auto dr = _window->getCurDeltaRot();
	return glm::vec2(dr[0], dr[1]);
}

glm::vec3 GameState::getTransDelta() const
{
	auto dt = _window->getCurDeltaTrans();
	return glm::vec3(dt[0], dt[1], dt[2]);
}

void GameState::updateRot()
{
	auto area = getRenderArea();
	_curRot += 500.0f * getRotDelta() / (float)area.z;
}

void GameState::updateTrans()
{
	glm::mat4 rotmat(1.0);
	rotmat = glm::rotate(rotmat, glm::radians(-_curRot.y - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotmat = glm::rotate(rotmat, glm::radians(-_curRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
	_curTrans += glm::mat3(rotmat) * getTransDelta() / 100.0f;
}

float GameState::getTime()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _startTime).count();
}

void GameState::update(VulkanDescriptorPool& pool, uint32_t frameID)
{
	int i = 0;
	for (auto& sd : _shaderData)
		sd->update(pool, frameID, i++, *this);
}
