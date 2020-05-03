#pragma once

#include <glm/glm.hpp>

#include "VulkanTypes.h"

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;

	static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};