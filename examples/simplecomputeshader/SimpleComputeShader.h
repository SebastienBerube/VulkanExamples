#ifndef SIMPLE_COMPUTE_SHADER_H_
#define SIMPLE_COMPUTE_SHADER_H_

#include "vulkan/vulkan.h"
#include "VulkanTexture.h"
#include "VulkanFramework.h"
#include "ShaderParser.h"
#include <string>
#include <vector>

namespace VulkanUtilities
{
    class SimpleComputeShader
    {
    public:
        
        //shaderAssetPath example : "computeshadernetwork/blur"
        SimpleComputeShader(VulkanFramework& framework, const std::string& shaderAssetPath);
        SimpleComputeShader(VulkanFramework& framework, const std::string& shaderAssetPath, const std::vector<UniformInfo>& uniforms, const std::vector<BindingInfo>& bindings);
        ~SimpleComputeShader();

        void SetFloat(const std::string& name, float val);
        void SetInt(const std::string& name, int val);
        void SetTexture(int kernelIndex, const std::string& name, ImageInfo imageInfo);
        void Dispatch(VkCommandBuffer commandBuffer, int kernelIndex, int frameIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ);
        void DispatchAllKernels_Test(VkCommandBuffer commandBuffer, int frameIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ, bool imageBarrier);
        void CreatePipeline();

    private:
        void UpdateUniforms(VkCommandBuffer commandBuffer);
        void PrepareDescriptorSets();

        //Not owner
        VulkanFramework& _framework;
        std::string _shaderAssetPath;

        //Owner
        std::vector<BindingInfo> _bindingInfos;
        std::vector<UniformInfo> _uniformInfos;
        std::vector<unsigned char> _uniformData;

        VkPushConstantRange _pushConstantRange;
        VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
        VkPipeline _pipeline = VK_NULL_HANDLE;
    };
}


#endif //SIMPLE_COMPUTE_SHADER_H_