#include "VulkanShader.h"

VulkanShader::VulkanShader(const VkDevice& device, std::string path, VkShaderStageFlagBits type)
{
	_readFile(path, bytes);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytes.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &_shader) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    _pssci = {};
    _pssci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _pssci.stage = type;
    _pssci.module = _shader;
    _pssci.pName = "main";
}

const VkShaderModule& VulkanShader::getShaderModule()
{
    return _shader;
}

const VkPipelineShaderStageCreateInfo& VulkanShader::getShaderStageCreateInfo()
{
    return _pssci;
}

void VulkanShader::destroy(const VkDevice& device)
{
    vkDestroyShaderModule(device, _shader, nullptr);
}

void VulkanShader::_readFile(std::string path, std::vector<char>& bytes)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open shader file: " + path);
    }

    size_t fileSize = (size_t)file.tellg();
    bytes.resize(fileSize);

    file.seekg(0);
    file.read(bytes.data(), fileSize);

    file.close();
}
