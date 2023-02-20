#ifndef UNITY_COMPUTE_SHADER_H_
#define UNITY_COMPUTE_SHADER_H_

#include "vulkan/vulkan.h"
#include "VulkanTexture.h"
#include "VulkanFramework.h"
#include <string>
#include <vector>

namespace VulkanUtilities
{

    class SimpleComputeShader
    {
    public:
        struct BindingInfo
        {
            std::string name;
            VkDescriptorType type;
            VkFormat format;
            uint32_t bindingIndex;
        };

        SimpleComputeShader(VulkanFramework& framework, const std::string& shader);
        ~SimpleComputeShader();

        void SetFloat(const std::string& name, float val);
        void SetInt(const std::string& name, int val);
        void SetTexture(int kernelIndex, const std::string& name, VkDescriptorImageInfo& textureDescriptor);
        void Dispatch(VkCommandBuffer commandBuffer, int kernelIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ);
        void CreatePipeline();

    private:
        void PrepareDescriptorSets();

        //Not owner
        VulkanFramework& _framework;
        std::string _shaderName;

        //Owner
        std::vector<BindingInfo> _bindingInfos;

        VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
        VkPipeline _pipeline = VK_NULL_HANDLE;
    };
}


#endif //UNITY_COMPUTE_SHADER_H_