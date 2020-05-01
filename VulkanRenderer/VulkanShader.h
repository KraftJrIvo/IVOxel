#pragma once

#include "VulkanTypes.h"

#include <fstream>

class VulkanShader
{
public:
	VulkanShader() = default;
	VulkanShader(std::string path, VkShaderStageFlagBits type);

	void compile();
	void create(const VkDevice& device);

	const VkShaderModule& getShaderModule();
	const VkPipelineShaderStageCreateInfo& getShaderStageCreateInfo();
	void destroy(const VkDevice& device);

private:
	std::string _path;
	VkShaderStageFlagBits _type;
	std::vector<char> bytes;
	VkShaderModule _shader;
	VkPipelineShaderStageCreateInfo _pssci;

	void _readFile(std::string path, std::vector<char>& bytes);
};