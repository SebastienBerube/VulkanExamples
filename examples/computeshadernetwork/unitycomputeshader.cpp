#include "unitycomputeshader.h"
#include "vulkanexamplebase.h"

namespace VulkanUtilities
{
    void PushBindind(std::vector<UnityComputeShader::BindingInfo>& bindingInfos, const std::string& name, VkDescriptorType type, VkFormat format)
    {
        bindingInfos.push_back(UnityComputeShader::BindingInfo{ name, type, format, (uint32_t)bindingInfos.size() });

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

    std::vector<VkDescriptorSetLayoutBinding> getDescriptorSetLayoutFromUnityShader(const std::string& shader)
    {
        std::vector<UnityComputeShader::BindingInfo> bindingInfos = readBindingInfosFromUnityShader(shader);

        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

        for (auto bindingInfo : bindingInfos)
        {
            setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(bindingInfo.type, VK_SHADER_STAGE_COMPUTE_BIT, bindingInfo.bindingIndex));
        }

        return setLayoutBindings;
    }

    UnityComputeShader::UnityComputeShader(VkDevice device, VkDescriptorPool descriptorPool, const std::string& shader)
    {
        _shaderName = shader;
        _device = device;
        _descriptorPool = descriptorPool;
        Prepare();
    }

    void UnityComputeShader::Prepare()
    {
        _bindingInfos = readBindingInfosFromUnityShader(_shaderName);

        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = getDescriptorSetLayoutFromUnityShader(_shaderName);

        VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(_device, &descriptorLayout, nullptr, &_descriptorSetLayout));

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(&_descriptorSetLayout, 1);

        VK_CHECK_RESULT(vkCreatePipelineLayout(_device, &pPipelineLayoutCreateInfo, nullptr, &_pipelineLayout));

        VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(_descriptorPool, &_descriptorSetLayout, 1);

        VK_CHECK_RESULT(vkAllocateDescriptorSets(_device, &allocInfo, &_descriptorSet));
    }

    UnityComputeShader::~UnityComputeShader()
    {

    }

    void UnityComputeShader::SetFloat(const std::string& name, float val)
    {

    }

    void UnityComputeShader::SetInt(const std::string& name, int val)
    {

    }

    void UnityComputeShader::SetTexture(int kernelIndex, const std::string& nameID, vks::Texture2D& texture)
    {
        //texture.descriptor
        auto it = std::find_if(_bindingInfos.begin(), _bindingInfos.end(), [&nameID](const BindingInfo& item)
            {
                return item.name == nameID;
            });

        BindingInfo bindingInfo = *it;

        //TODO : Log error here.

        std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
            vks::initializers::writeDescriptorSet(_descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, bindingInfo.bindingIndex, &texture.descriptor)
        };
        vkUpdateDescriptorSets(_device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);
    }

    void UnityComputeShader::Dispatch(int kernelIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ)
    {

    }
}


