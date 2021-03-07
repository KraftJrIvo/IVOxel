#include "Camera.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

void Camera::translate(glm::vec3 delta)
{
	pos += delta;
}

void Camera::rotate(glm::vec3 delta)
{
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::rotate(view, glm::radians(delta.y), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, glm::radians(delta.x), glm::vec3(0.0f, 1.0f, 0.0f));

	auto proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	proj[1][1] *= -1;

	mvp = proj * view;
}
