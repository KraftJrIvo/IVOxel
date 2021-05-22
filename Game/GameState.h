#pragma once

#include <list>
#include <chrono>

#include <VoxelMap.h>

#include "GameData.h"
#include "Camera.h"

class Window;

enum GameDataType : unsigned int
{
	CAMERA = 0,
	LIGHTING = 1,
	LOCAL_MAP = 2,
	CONSTANTS = 3
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

	const std::shared_ptr<GameData> getGameData(uint8_t key) const;
	const glm::vec2& getRot() { return _curRot; }
	const glm::vec3& getTrans() { return _cam.pos; }
	VoxelMap& getMap() { return _map; }
	float getTime();
	Camera& getCam();
	
	void update(uint8_t group, GameDataContainer* container = nullptr, uint32_t frameID = 0);

private:
	VoxelMap& _map;

	std::map<uint8_t, std::shared_ptr<GameData>> _gameData;

	Camera _cam;
	glm::vec3 _curRot;

	std::chrono::steady_clock::time_point _startTime;
};