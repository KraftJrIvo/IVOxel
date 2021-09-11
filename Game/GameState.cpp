#include "Window.h"

#include "GameState.h"

#include "GameDataCamera.h"
#include "GameDataLight.h"
#include "GameDataMap.h"
#include "GameDataConst.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

GameState::GameState(Window* window, VoxelMap& map) : 
	_map(map),
	_window(window)
{ 
	_cam.fov = 90.0f;
	auto ra = getRenderArea();
	_cam.res = { ra[2], ra[3] };
	_curRot = glm::vec3(0.0f, 180.0f, 0.0f);
	_cam.rotate(_curRot);
	_cam.translate(glm::vec3(0.51f, 1.51f, 0.51f));
	
	_startTime = std::chrono::high_resolution_clock::now();

	_gameData = { 
		{CAMERA, std::make_shared<GameDataCamera>()},
		{LIGHTING, std::make_shared<GameDataLight>(map.getMaxLights())},
		{LOCAL_MAP, std::make_shared<GameDataMap>(map.getMapDataSize(true))},
		{LOCAL_MAP2, std::make_shared<GameDataMap>(map.getMapDataSize(true))},
		{CONSTANTS, std::make_shared<GameDataConst>(map.getLoadRadius(), map.getMaxLights(), 0.00001)}
	};
}

glm::vec4 GameState::getRenderArea() const
{
	auto ra = _window->getRenderArea();
	return ra;
}

glm::vec3 GameState::getRotDelta() const
{
	auto dr = _window->getCurDeltaRot();
	return glm::vec3(dr[0], dr[1], 0);
}

glm::vec3 GameState::getTransDelta() const
{
	auto dt = _window->getCurDeltaTrans();
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

const std::list<std::shared_ptr<GameData>> GameState::getGameData() const
{
	std::list<std::shared_ptr<GameData>> list;
	for (auto& gd : _gameData) {
		list.push_back(gd.second);
	}
	return list;
}

const std::shared_ptr<GameData> GameState::getGameData(uint8_t key) const
{
	if (_gameData.count(key)) 
	{
		return _gameData.at(key);
	}
	return nullptr;
}

const std::vector<uint8_t>& GameState::getLightData() const
{
	return _lightData;
}

const std::vector<uint8_t>& GameState::getMapData() const
{
	return _mapData;
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

void GameState::startUpdateLoop(GameDataContainer* container, uint8_t nFrames)
{
	std::vector<int32_t> pos = { (int)floor(_cam.pos.x), (int)floor(_cam.pos.y), (int)floor(_cam.pos.z) };
	_lightData = _map.getLightDataAt(pos, _map.getLoadRadius(), getTime());
	_mapData = _map.getChunksData(_map.getLoadRadius(), true);
	update(ALL_GROUPS);
	for (uint8_t i = 0; i < nFrames; ++i)
		upload(ALL_GROUPS, container, i);

	static bool firstTime = true;

	_cam.mapId = 0;

	while (true)
	{
		{
			std::lock_guard lock(updMtx);

			update(EVERY_FRAME);

			static glm::vec3 abspos = { _map.getAbsPos()[0],_map.getAbsPos()[1],_map.getAbsPos()[2] };

			pos = { (int)floor(_cam.pos.x), (int)floor(_cam.pos.y), (int)floor(_cam.pos.z) };
			_lightData = _map.getLightDataAt(pos, _map.getLoadRadius(), getTime());

			if (_map.checkAndLoad(pos, false) || firstTime)
			{
				_cam.mapId = !_cam.mapId;
				abspos = { _map.getAbsPos()[0],_map.getAbsPos()[1],_map.getAbsPos()[2] };
				_mapData = _map.getChunksData(_map.getLoadRadius(), true);
				upload(EVERY_LOAD, container, 0, _cam.mapId ? LOCAL_MAP2 : LOCAL_MAP);
				firstTime = false;
			}
			_cam.abspos = _cam.pos - abspos;

			for (uint8_t i = 0; i < nFrames; ++i)
				upload(EVERY_FRAME, container, i);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void GameState::update(uint8_t group, int32_t gdID)
{
	for (auto& gd : _gameData)
		if ((gd.second->updateGroup == group && (gdID == -1 || gd.first == gdID)) || group == ALL_GROUPS)
		{
			gd.second->update(*this, gd.first);
		}
}

void GameState::upload(uint8_t group, GameDataContainer* container, uint32_t frameID, int32_t gdID)
{
	for (auto& gd : _gameData)
		if ((gd.second->updateGroup == group && (gdID == -1 || gd.first == gdID)) || group == ALL_GROUPS)
		{
			gd.second->upload(*this, gd.first, container, frameID);
		}
}

