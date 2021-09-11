#pragma once

#include <list>
#include <chrono>

#include <VoxelMap.h>

#include "GameData.h"
#include "Camera.h"
#include "Window.h"

class Window;

enum GameDataType : unsigned int
{
	CAMERA = 0,
	LIGHTING = 1,
	LOCAL_MAP = 2,
	LOCAL_MAP2 = 3,
	CONSTANTS = 4
};

class GameState
{
public:
	GameState(Window* window, VoxelMap& map);
	
	glm::vec4 getRenderArea() const;
	glm::vec3 getRotDelta() const;
	glm::vec3 getTransDelta() const;
	
	void updateRot();
	void updateTrans();

	const std::list<std::shared_ptr<GameData>> getGameData() const;
	const std::shared_ptr<GameData> getGameData(uint8_t key) const;
	const std::vector<uint8_t>& getLightData() const;
	const std::vector<uint8_t>& getMapData() const;
	const glm::vec2& getRot() { return _curRot; }
	const glm::vec3& getTrans() { return _cam.pos; }
	VoxelMap& getMap() { return _map; }
	float getTime();
	Camera& getCam();

	void startUpdateLoop(GameDataContainer* container, uint8_t nFrames);
	
	void update(uint8_t group, int32_t gdID = -1);
	void upload(uint8_t group, GameDataContainer* container = nullptr, uint32_t frameID = 0, int32_t gdID = -1);

	std::mutex updMtx;

private:
	VoxelMap& _map;

	std::map<uint8_t, std::shared_ptr<GameData>> _gameData;

	std::vector<uint8_t> _lightData;
	std::vector<uint8_t> _mapData;

	Window* _window;
	Camera _cam;
	glm::vec3 _curRot;

	std::chrono::steady_clock::time_point _startTime;
};