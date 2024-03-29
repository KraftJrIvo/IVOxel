#pragma once

#include <glm/glm.hpp>

struct Camera
{
public:

	Camera() : pos(glm::vec3(0)), res(glm::vec2(0)), fov(0)
	{
		rotate(glm::vec3(0));
	}
	
	Camera(glm::vec2 resolution, float fieldOfView, glm::vec3 translation, glm::vec3 rotation) : pos(translation), res(resolution), fov(fieldOfView)
	{ 
		rotate(rotation);
	}

	glm::mat4 mvp;
	glm::vec2 res;
	float	  fov;
	uint32_t  mapId;
	glm::vec3 pos;
	float     _align2_; //for gpu alignment
	glm::vec3 abspos;

	void translate(glm::vec3 delta);
	void rotate(glm::vec3 delta);
};
