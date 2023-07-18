#include "vulkan/vulkan.h"
#include "VulkanFramework.h"
#include <string>

namespace VulkanUtilities
{
    VkPipelineShaderStageCreateInfo VulkanExampleFramework::loadShader(std::string fileName, VkShaderStageFlagBits stage)
    {
        return _base.loadShader(fileName, stage);
    }

    std::string VulkanExampleFramework::getShadersPath() const
    {
        switch (_base.settings.shadingLang)
        {
        case ShadingLanguage::GLSL:
            return getShaderBasePath() + "/glsl/";
        case ShadingLanguage::HLSL:
            return getShaderBasePath() + "/hlsl/";
        default:
            std::cerr << "Error: Unknown shader file type." << "\n";
            return "";
        }
    }

    VkDevice VulkanExampleFramework::getVkDevice() const
    {
        return _base.vulkanDevice->logicalDevice;
    }

    VkDescriptorPool VulkanExampleFramework::getDescriptorPool() const
    {
        return _descriptorPool;
    }

    VkPipelineCache VulkanExampleFramework::getPipelineCache() const
    {
        return _pipelineCache;
    }

    VulkanExampleFramework::VulkanExampleFramework(VulkanExampleBase& exampleBase, VkDescriptorPool descriptorPool, VkPipelineCache pipelineCache)
        : _base(exampleBase)
        , _descriptorPool(descriptorPool)
        , _pipelineCache(pipelineCache)
    {
    }
}
