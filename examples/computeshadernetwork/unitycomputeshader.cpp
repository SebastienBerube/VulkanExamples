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
        //TODO : Read compute shader file, auto-generate this list
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            // F (external forces)
            // Texture2D<float2> F_in;
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),

            // U (velocity field)
            // Texture2D<float2> U_in;
            // SamplerState samplerU_in;
            // RWTexture2D<float2> U_out;
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 2),

            // W (velocity field; working)
            // Texture2D<float2> W_in;
            // RWTexture2D<float2> W_out;
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 3),
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 4),

            // Div W
            // RWTexture2D<float> DivW_out;
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 5),

            // P (pressure field)
            // Texture2D<float> P_in;
            // RWTexture2D<float> P_out;
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 6),
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 7),

            // Obstacles map
            // Texture2D<float> O_in;
            // SamplerState samplerO_in;
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, 8),

            //Texture2D<float> X1_in;
            //Texture2D<float> B1_in;
            //RWTexture2D<float> X1_out;
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 9),
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 10),
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 11),

            //Texture2D<float2> X2_in;
            //Texture2D<float2> B2_in;
            //RWTexture2D<float2> X2_out;
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 12),
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 13),
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 14),

            //Texture2D<float> VOR_in;
            //RWTexture2D<float> VOR_out;
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 15),
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 16),
        };
    }

    void setupComputeDescriptorSets(
        VkDevice device,
        VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout& descriptorSetLayout,
        VkPipelineLayout& pipelineLayout,
        VkDescriptorSet& descriptorSet)
    {
        //getDescriptorSetLayoutFromUnityShader
        //= computePass.descriptorSetLayout;
        /*
        VkPipelineLayout& pipelineLayout = computePass.pipelineLayout;
        VkDescriptorSet& descriptorSet = computePass.descriptorSet;
        VkDescriptorImageInfo& dstImageDescriptor = computePass.textureComputeTarget.descriptor;
        */
        //C:\Dev\Perso\GitHub\Bers\VulkanExamples\data\shaders\unity


        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = getDescriptorSetLayoutFromUnityShader("FluidSimCommon.compute");
        
        VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);

        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));

        VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

        //TODO : Externalize this?
        /*
        std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
            vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &srcImageDescriptor),
            vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &dstImageDescriptor)
        };
        vkUpdateDescriptorSets(device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);
        */
    }

    UnityComputeShader::UnityComputeShader(const std::string& shader)
    {
        //buildCommandBuffers
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
        std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
            vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &srcImageDescriptor),
            vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &dstImageDescriptor)
        };
    }

    void UnityComputeShader::Dispatch(int kernelIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ)
    {

    }
}


