#pragma once

#include "VulkanTypes.h"

#include <fstream>

class VulkanShader
{
public:
	VulkanShader() = default;
	VulkanShader(const VkDevice& device, std::string path, VkShaderStageFlagBits type);

	const VkShaderModule& getShaderModule();
	void destroy(const VkDevice& device);

private:
	std::vector<char> bytes;
	VkShaderModule _shader;
	VkPipelineShaderStageCreateInfo _pssci;

	void _readFile(std::string path, std::vector<char>& bytes);
};