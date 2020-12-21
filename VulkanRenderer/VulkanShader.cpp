#include "VulkanShader.h"

#include <iostream>
#include <shaderc/shaderc.hpp>

VulkanShader::VulkanShader(std::string path, VkShaderStageFlagBits type) :
    _path(path),
    _type(type)
{
}

void VulkanShader::compile()
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    _readFile(_path, bytes);
    bytes.push_back(0);

    // Like -DMY_DEFINE=1
    options.AddMacroDefinition("MY_DEFINE", "1");
    options.SetOptimizationLevel(shaderc_optimization_level_size);

    shaderc_shader_kind kind = (_type == VK_SHADER_STAGE_VERTEX_BIT) ? shaderc_vertex_shader : shaderc_fragment_shader;

    shaderc::SpvCompilationResult module =
        compiler.CompileGlslToSpv(bytes.data(), kind, _path.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cout << module.GetErrorMessage();
    }

    std::vector<uint32_t> result = { module.cbegin(), module.cend() };

    size_t sz = result.size() * sizeof(uint32_t);
    bytes.resize(sz);
    std::memcpy(bytes.data(), result.data(), sz);
}

void VulkanShader::create(const VkDevice& device)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytes.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &_shader) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    _pssci = {};
    _pssci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _pssci.stage = _type;
    _pssci.module = _shader;
    _pssci.pName = "main";
}

const VkShaderModule& VulkanShader::getShaderModule()
{
    return _shader;
}

const VkPipelineShaderStageCreateInfo& VulkanShader::getShaderStageCreateInfo() const
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
