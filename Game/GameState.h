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
	void init(Window* window);
	
	void setMap(const VoxelMap& map);
	
	glm::vec4 getRenderArea() const;
	glm::vec3 getRotDelta() const;
	glm::vec3 getTransDelta() const;
	
	void updateRot();
	void updateTrans();

	const std::list<std::shared_ptr<GameData>>& getGameData() { return _GameData; }
	const glm::vec2& getRot() { return _curRot; }
	const glm::vec3& getTrans() { return _cam.pos; }
	VoxelMap& getMap() { return _map; }
	float getTime();
	Camera& getCam();
	
	void update(GameDataContainer* container, uint32_t frameID);

private:
	Window* _window;
	VoxelMap _map;

	std::list<std::shared_ptr<GameData>> _GameData;

	Camera _cam;
	glm::vec3 _curRot;

	std::chrono::steady_clock::time_point _startTime;
};