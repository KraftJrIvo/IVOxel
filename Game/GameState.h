#pragma once

#include <list>
#include <chrono>

#include <VoxelMap.h>

#include "GameData.h"
#include "Camera.h"

class Window;

class GameState
{
public:
	void init(Window* window, float lightRadius);
	
	void setMap(VoxelMap* map);
	
	glm::vec4 getRenderArea() const;
	glm::vec3 getRotDelta() const;
	glm::vec3 getTransDelta() const;
	
	void updateRot();
	void updateTrans();

	const std::list<std::shared_ptr<GameData>>& getGameData() { return _gameData; }
	const glm::vec2& getRot() { return _curRot; }
	const glm::vec3& getTrans() { return _cam.pos; }
	VoxelMap* getMap() { return _map; }
	float getTime();
	Camera& getCam();
	
	void update(GameDataContainer* container, uint32_t frameID);

private:
	VoxelMap* _map = nullptr;

	std::list<std::shared_ptr<GameData>> _gameData;

	Camera _cam;
	glm::vec3 _curRot;

	std::chrono::steady_clock::time_point _startTime;
};