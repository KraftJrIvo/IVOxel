#pragma once

#include <map>

#include "VulkanDevice.h"
#include "VulkanShader.h"

enum class ShaderType
{
	VERTEX,
	FRAGMENT
};

class VulkanShaderManager
{
public:
	void destroy(const VulkanDevice& device);

	void addShader(const VulkanDevice& device, ShaderType type, std::string name);
	const VulkanShader& getShader(ShaderType type, std::string name);
	std::vector<VkPipelineShaderStageCreateInfo> getShaderStageCreateInfos(const std::vector<std::pair<ShaderType, std::string>>& shaderTypesAndNames);
private:
	std::map<std::string, VulkanShader>   _vertexShaders;
	std::map<std::string, VulkanShader> _fragmentShaders;
	std::vector<VkPipelineShaderStageCreateInfo> _shaderInfos;
};