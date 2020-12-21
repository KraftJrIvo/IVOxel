#include "ShaderDataCamera.h"

#include "VulkanDescriptorPool.h"
#include "GameState.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

void ShaderDataCamera::update(VulkanDescriptorPool& pool, uint32_t frameID, uint32_t dataID, GameState& game)
{
	game.updateRot();
	game.updateTrans();

	auto& rot = game.getRot();
	auto& trans = game.getTrans();

	auto model = glm::mat4(1.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::rotate(view, glm::radians(rot.y), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, glm::radians(rot.x), glm::vec3(0.0f, 1.0f, 0.0f));

	auto renderArea = game.getRenderArea();
	auto w = renderArea[2];
	auto h = renderArea[3];
	auto proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	proj[1][1] *= -1;

	state.mvp = proj * view;
	state.time = game.getTime();
	state.resolution = { w, h };
	state.fov = 90.0f;
	state.pos = trans;

	pool.setData(frameID, dataID, &state);
}
