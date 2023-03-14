#include "SimpleComputeShader.h"
#include "vulkanexamplebase.h"

#include <regex>
#include <vector>
#include <iostream>

namespace VulkanUtilities
{
    std::vector<VkDescriptorSetLayoutBinding> getDescriptorSetLayout(const std::vector<BindingInfo>& bindingInfos)
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

        for (auto bindingInfo : bindingInfos)
        {
            setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(bindingInfo.type, VK_SHADER_STAGE_COMPUTE_BIT, bindingInfo.bindingIndex));
        }

        return setLayoutBindings;
    }

    SimpleComputeShader::SimpleComputeShader(VulkanFramework& framework, const std::string& shader)
        : _framework(framework)
    {
        _uniformInfos = ParseShaderUniforms(shader);
        _shaderName = shader;
        int totalSize = GetTotalSize(_uniformInfos);
        _uniformData.resize(totalSize, (unsigned char)0);
        /*SetInt("kernelIndex", 255);
        SetInt("frameIndex", 256*255+255);
        SetInt("frameIndex", 256 * 255 + 255);*/

        PrepareDescriptorSets();
        //CreatePipeline(); TODO : Test this here, during construction.
    }

    SimpleComputeShader::~SimpleComputeShader()
    {
        VkDevice device = _framework.getVkDevice();

        //Note : Previously, all compute shader pipelines were deleted first, then 
        //       all vkDestroyPipelineLayout and vkDestroyDescriptorSetLayout in a 2nd loop.
        vkDestroyPipeline(device, _pipeline, nullptr);
        vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);
    }

    void SimpleComputeShader::PrepareDescriptorSets()
    {
        _bindingInfos = ParseShaderBindings(_shaderName);

        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = getDescriptorSetLayout(_bindingInfos);

        VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(_framework.getVkDevice(), &descriptorLayout, nullptr, &_descriptorSetLayout));

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(&_descriptorSetLayout, 1);
        
        //<ADDED S.B. Push_Contants>
        _pushConstantRange.offset = 0;
        _pushConstantRange.size = GetTotalSize(_uniformInfos);
        //_pushConstantRange.size = sizeof(PushConstants);
        _pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pPipelineLayoutCreateInfo.pPushConstantRanges = &_pushConstantRange;
        //</ADDED S.B. Push_Contants>

        VK_CHECK_RESULT(vkCreatePipelineLayout(_framework.getVkDevice(), &pPipelineLayoutCreateInfo, nullptr, &_pipelineLayout));

        VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(_framework.getDescriptorPool(), &_descriptorSetLayout, 1);

        VK_CHECK_RESULT(vkAllocateDescriptorSets(_framework.getVkDevice(), &allocInfo, &_descriptorSet));
    }

    void SimpleComputeShader::CreatePipeline()
    {
        // Create compute shader pipelines
        VkComputePipelineCreateInfo computePipelineCreateInfo =
            vks::initializers::computePipelineCreateInfo(_pipelineLayout, 0);

        std::string fileName = _framework.getShadersPath() + "computeshadernetwork/" + _shaderName + ".comp.spv";
        computePipelineCreateInfo.stage = _framework.loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);

        VK_CHECK_RESULT(vkCreateComputePipelines(_framework.getVkDevice(), _framework.getPipelineCache(), 1, &computePipelineCreateInfo, nullptr, &_pipeline));
    }

    void SimpleComputeShader::SetFloat(const std::string& name, float val)
    {
        SetValue(_uniformInfos, _uniformData, name, &val);
    }

    void SimpleComputeShader::SetInt(const std::string& name, int val)
    {
        SetValue(_uniformInfos, _uniformData, name, &val);
    }

    void SimpleComputeShader::SetTexture(int kernelIndex, const std::string& nameID, VkDescriptorImageInfo& textureDescriptor)
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


    void SimpleComputeShader::Dispatch(VkCommandBuffer commandBuffer, int kernelIndex, int frameIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ)
    {
        pc.kernelIndex = kernelIndex;
        pc.frameIndex += 1;
        
        vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &pc);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1, &_descriptorSet, 0, 0);
        vkCmdDispatch(commandBuffer, threadGroupsX, threadGroupsY, threadGroupsZ);
    }

    void SimpleComputeShader::DispatchAllKernels_Test(VkCommandBuffer commandBuffer, int frameIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1, &_descriptorSet, 0, 0);
        
        pc.frameIndex += 1;
        //vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, GetTotalSize(_uniformInfos), &_uniformData[0]);
        SetFloat("alpha", 0.663);
        SetInt("frameIndex", pc.frameIndex);
        SetInt("kernelIndex", 0);
        vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, GetTotalSize(_uniformInfos), &_uniformData[0]);

        vkCmdDispatch(commandBuffer, threadGroupsX, threadGroupsY, threadGroupsZ);

        SetInt("kernelIndex", 1);
        vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, GetTotalSize(_uniformInfos), &_uniformData[0]);

        vkCmdDispatch(commandBuffer, threadGroupsX, threadGroupsY, threadGroupsZ);

        SetInt("kernelIndex", 2);
        vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, GetTotalSize(_uniformInfos), &_uniformData[0]);

        vkCmdDispatch(commandBuffer, threadGroupsX, threadGroupsY, threadGroupsZ);
    }
}


