#include "Window.h"

#include "GameState.h"

#include "GameDataCamera.h"
#include "GameDataLight.h"
#include "GameDataMap.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

void GameState::init(Window* window, float lightRadius)
{ 
	_cam.window = window;
	_cam.fov = 90.0f;
	auto ra = getRenderArea();
	_cam.res = { ra[2], ra[3] };
	_curRot = glm::vec3(0.0f, 180.0f, 0.0f);
	_cam.rotate(_curRot);
	_cam.translate(glm::vec3(0.5f, 0.5f, 0.5f));
	
	_startTime = std::chrono::high_resolution_clock::now();

	_gameData = { std::make_shared<GameDataCamera>(), std::make_shared<GameDataLight>(lightRadius)/*, std::make_shared<GameDataMap>()*/ };
}

void GameState::setMap(VoxelMap* map)
{
	_map = map;
}

glm::vec4 GameState::getRenderArea() const
{
	auto ra = _cam.window->getRenderArea();
	return ra;
}

glm::vec3 GameState::getRotDelta() const
{
	auto dr = _cam.window->getCurDeltaRot();
	return glm::vec3(dr[0], dr[1], 0);
}

glm::vec3 GameState::getTransDelta() const
{
	auto dt = _cam.window->getCurDeltaTrans();
	return glm::vec3(dt[0], dt[1], dt[2]);
}

void GameState::updateRot()
{
	auto area = getRenderArea();
	auto delta = 500.0f * getRotDelta() / (float)area.z;
	_curRot += delta;
	_cam.rotate(_curRot);
}

void GameState::updateTrans()
{
	glm::mat4 rotmat(1.0);
	rotmat = glm::rotate(rotmat, glm::radians(-_curRot.y - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotmat = glm::rotate(rotmat, glm::radians(-_curRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
	_cam.translate(glm::mat3(rotmat) * getTransDelta() / 100.0f);
}

float GameState::getTime()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _startTime).count();
}

Camera& GameState::getCam()
{
	return _cam;
}

void GameState::update(GameDataContainer* container, uint32_t frameID)
{
	int i = 0;
	for (auto& sd : _gameData)
		sd->update(container, frameID, i++, *this);
}
