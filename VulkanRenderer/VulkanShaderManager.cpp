#include "VulkanShaderManager.h"

void VulkanShaderManager::destroy(const VulkanDevice& device)
{
	for (auto& shader : _vertexShaders)
		shader.second.destroy(device.get());

	for (auto& shader : _fragmentShaders)
		shader.second.destroy(device.get());
}

void VulkanShaderManager::addShader(const VulkanDevice& device, ShaderType type, std::string name)
{
	VulkanShader shader;
	switch (type)
	{
	case ShaderType::VERTEX:
		shader = VulkanShader(name + ".vert", VK_SHADER_STAGE_VERTEX_BIT);
		break;
	case ShaderType::FRAGMENT:
		shader = VulkanShader(name + ".frag", VK_SHADER_STAGE_FRAGMENT_BIT);
		break;
	default:
		break;
	}

	shader.compile();
	shader.create(device.get());

	switch (type)
	{
	case ShaderType::VERTEX:
		_vertexShaders[name] = shader;
		break;
	case ShaderType::FRAGMENT:
		_fragmentShaders[name] = shader;
		break;
	default:
		break;
	}
}

const VulkanShader& VulkanShaderManager::getShader(ShaderType type, std::string name)
{
	switch (type)
	{
	case ShaderType::VERTEX:
		return _vertexShaders[name];
	case ShaderType::FRAGMENT:
		return _fragmentShaders[name];
	}
}

std::vector<VkPipelineShaderStageCreateInfo> VulkanShaderManager::getShaderStageCreateInfos(const std::vector<std::pair<ShaderType, std::string>>& shaderTypesAndNames)
{
	std::vector<VkPipelineShaderStageCreateInfo> res;

	for (auto& stan : shaderTypesAndNames)
		res.push_back(getShader(stan.first, stan.second).getShaderStageCreateInfo());
	
	return res;
}
