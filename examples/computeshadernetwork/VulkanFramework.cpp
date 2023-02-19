#include "vulkan/vulkan.h"
#include <string>
#include <functional>
#include "VulkanFramework.h"

namespace VulkanUtilities
{
    VkPipelineShaderStageCreateInfo VulkanExampleFramework::loadShader(std::string fileName, VkShaderStageFlagBits stage)
    {
        return _base.loadShader(fileName, stage);
    }

    std::string VulkanExampleFramework::getShadersPath() const
    {
        const std::string shaderDir = "glsl";
        return getAssetPath() + "shaders/" + shaderDir + "/";
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
