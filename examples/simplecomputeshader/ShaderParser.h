#ifndef SHADER_PARSER_H_
#define SHADER_PARSER_H_

#include <vector>
#include <string>
#include "vulkan/vulkan.h"
#include "VulkanTexture.h"
#include "VulkanFramework.h"

namespace VulkanUtilities
{
    enum UniformType
    {
        FLOAT = 0,
        INT = 1,
        UNSUPPORTED = 2
    };

    struct UniformInfo
    {
        std::string name;
        UniformType type;
        int order;
        int byteOffset;
    };

    struct BindingInfo
    {
        std::string name;
        VkDescriptorType type;
        VkFormat format;
        uint32_t bindingIndex;
    };

    std::vector<BindingInfo> ParseShaderBindings(const std::string& shader);

    std::vector<UniformInfo> ParseShaderUniforms(const std::string& shader);

    int GetTypeSize(UniformType type);

    int GetTotalSize(std::vector<UniformInfo>& uniforms);

    void SetValue(const std::vector<UniformInfo>& uniformInfos, std::vector<unsigned char>& uniformData, std::string name, void* src);

    std::vector<UniformInfo> GetGlobalVariables();
}

#endif //SHADER_PARSER_H_