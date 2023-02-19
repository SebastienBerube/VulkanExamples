#ifndef VULKAN_FRAMEWORK_H_
#define VULKAN_FRAMEWORK_H_

#include "vulkan/vulkan.h"
#include "vulkanexamplebase.h"
#include <string>
//#include <functional>

namespace VulkanUtilities
{
    class VulkanFramework
    {
    public:
        virtual VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage) = 0;
        virtual std::string getShadersPath() const = 0;
        virtual VkDevice getVkDevice() const = 0;
        virtual VkDescriptorPool getDescriptorPool() const = 0;
        virtual VkPipelineCache getPipelineCache() const = 0;
    };

    class VulkanExampleFramework : public VulkanFramework
    {
        VulkanExampleBase& _base;
        VkDescriptorPool _descriptorPool;
        VkPipelineCache _pipelineCache;

    public:
        VulkanExampleFramework(VulkanExampleBase& exampleBase, VkDescriptorPool descriptorPool, VkPipelineCache pipelineCache);
        
        VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);
        std::string getShadersPath() const;
        
        VkDevice getVkDevice() const;
        VkDescriptorPool getDescriptorPool() const;
        VkPipelineCache getPipelineCache() const;
    };
}

#endif //VULKAN_FRAMEWORK_H_