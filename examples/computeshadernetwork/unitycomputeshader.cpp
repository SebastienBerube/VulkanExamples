#include "unitycomputeshader.h"
#include "vulkanexamplebase.h"

namespace VulkanUtilities
{
    void PushBindind(std::vector<UnityComputeShader::BindingInfo>& bindingInfos, const std::string& name, VkDescriptorType type, VkFormat format)
    {
        bindingInfos.push_back(UnityComputeShader::BindingInfo{ name, type, format, (uint32_t)bindingInfos.size() });

    }

    std::vector<UnityComputeShader::BindingInfo> readBindingInfosFromExampleShaders (const std::string& shader)
    {
        std::vector<UnityComputeShader::BindingInfo> bindingInfos;
        PushBindind(bindingInfos, "inputImage", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        PushBindind(bindingInfos, "resultImage", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        return bindingInfos;
    }

    std::vector<UnityComputeShader::BindingInfo> readBindingInfosFromUnityShader(const std::string& shader)
    {
        std::vector<UnityComputeShader::BindingInfo> bindingInfos;
        
        // F (external forces)
        // Texture2D<float2> F_in;
        PushBindind(bindingInfos, "F_in", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32G32_SFLOAT);

        // U (velocity field)
        // Texture2D<float2> U_in;
        // SamplerState samplerU_in;
        PushBindind(bindingInfos, "U_in", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_FORMAT_R32G32_SFLOAT);

        // RWTexture2D<float2> U_out;
        PushBindind(bindingInfos, "U_out", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32G32_SFLOAT);

        // W (velocity field; working)
        // Texture2D<float2> W_in;
        PushBindind(bindingInfos, "W_in", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32G32_SFLOAT);

        // RWTexture2D<float2> W_out;
        PushBindind(bindingInfos, "W_out", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32G32_SFLOAT);

        // Div W
        // RWTexture2D<float> DivW_out;
        PushBindind(bindingInfos, "DivW_out", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32_SFLOAT);

        // P (pressure field)
        // Texture2D<float> P_in;
        PushBindind(bindingInfos, "P_in", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32_SFLOAT);

        // RWTexture2D<float> P_out;
        PushBindind(bindingInfos, "P_out", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32_SFLOAT);

        // Obstacles map
        // Texture2D<float> O_in;
        // SamplerState samplerO_in;
        PushBindind(bindingInfos, "O_in", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_FORMAT_R32_SFLOAT);

        // Texture2D<float> X1_in;
        PushBindind(bindingInfos, "X1_in", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32_SFLOAT);

        // Texture2D<float> B1_in;
        PushBindind(bindingInfos, "B1_in", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32_SFLOAT);

        // RWTexture2D<float> X1_out;
        PushBindind(bindingInfos, "X1_out", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32_SFLOAT);

        // Texture2D<float2> X2_in;
        PushBindind(bindingInfos, "X2_in", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32G32_SFLOAT);

        // Texture2D<float2> B2_in;
        PushBindind(bindingInfos, "B2_in", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32G32_SFLOAT);

        // RWTexture2D<float2> X2_out;
        PushBindind(bindingInfos, "X2_out", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32G32_SFLOAT);

        // Texture2D<float> VOR_in;
        PushBindind(bindingInfos, "VOR_in", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32_SFLOAT);

        // RWTexture2D<float> VOR_out;
        PushBindind(bindingInfos, "VOR_out", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32_SFLOAT);
        
        return bindingInfos;
    }

    std::vector<VkDescriptorSetLayoutBinding> getDescriptorSetLayout(const std::vector<UnityComputeShader::BindingInfo>& bindingInfos)
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

        for (auto bindingInfo : bindingInfos)
        {
            setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(bindingInfo.type, VK_SHADER_STAGE_COMPUTE_BIT, bindingInfo.bindingIndex));
        }

        return setLayoutBindings;
    }

    UnityComputeShader::UnityComputeShader(VulkanFramework& framework, const std::string& shader)
        : _framework(framework)
    {
        _shaderName = shader;
        PrepareDescriptorSets();
        //CreatePipeline(); TODO : Test this here, during construction.
    }

    UnityComputeShader::~UnityComputeShader()
    {
        VkDevice device = _framework.getVkDevice();

        //Note : Previously, all compute shader pipelines were deleted first, then 
        //       all vkDestroyPipelineLayout and vkDestroyDescriptorSetLayout in a 2nd loop.
        vkDestroyPipeline(device, _pipeline, nullptr);
        vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);
    }

    void UnityComputeShader::PrepareDescriptorSets()
    {
        _bindingInfos = readBindingInfosFromExampleShaders(_shaderName);

        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = getDescriptorSetLayout(_bindingInfos);

        VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(_framework.getVkDevice(), &descriptorLayout, nullptr, &_descriptorSetLayout));

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(&_descriptorSetLayout, 1);

        VK_CHECK_RESULT(vkCreatePipelineLayout(_framework.getVkDevice(), &pPipelineLayoutCreateInfo, nullptr, &_pipelineLayout));

        VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(_framework.getDescriptorPool(), &_descriptorSetLayout, 1);

        VK_CHECK_RESULT(vkAllocateDescriptorSets(_framework.getVkDevice(), &allocInfo, &_descriptorSet));
    }

    void UnityComputeShader::CreatePipeline()
    {
        // Create compute shader pipelines
        VkComputePipelineCreateInfo computePipelineCreateInfo =
            vks::initializers::computePipelineCreateInfo(_pipelineLayout, 0);

        std::string fileName = _framework.getShadersPath() + "computeshadernetwork/" + _shaderName + ".comp.spv";
        computePipelineCreateInfo.stage = _framework.loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);

        VK_CHECK_RESULT(vkCreateComputePipelines(_framework.getVkDevice(), _framework.getPipelineCache(), 1, &computePipelineCreateInfo, nullptr, &_pipeline));
    }

    void UnityComputeShader::SetFloat(const std::string& name, float val)
    {

    }

    void UnityComputeShader::SetInt(const std::string& name, int val)
    {

    }

    void UnityComputeShader::SetTexture(int kernelIndex, const std::string& nameID, VkDescriptorImageInfo& textureDescriptor)
    {
        auto it = std::find_if(_bindingInfos.begin(), _bindingInfos.end(), [&nameID](const BindingInfo& item)
            {
                return item.name == nameID;
            });

        BindingInfo bindingInfo = *it;

        //TODO : Log error here.
        std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
            vks::initializers::writeDescriptorSet(_descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, bindingInfo.bindingIndex, &textureDescriptor)
        };
        vkUpdateDescriptorSets(_framework.getVkDevice(), computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);
    }

    void UnityComputeShader::Dispatch(VkCommandBuffer commandBuffer, int kernelIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1, &_descriptorSet, 0, 0);
        vkCmdDispatch(commandBuffer, threadGroupsX, threadGroupsY, threadGroupsZ);
    }
}


