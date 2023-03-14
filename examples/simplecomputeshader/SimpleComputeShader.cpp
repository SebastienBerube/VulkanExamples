#include "SimpleComputeShader.h"
#include "vulkanexamplebase.h"

#include <regex>
#include <vector>
#include <iostream>

namespace VulkanUtilities
{
    void ImageBarrier(VkCommandBuffer commandBuffer, VkImage image)
    {
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageMemoryBarrier.html
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        // We won't be changing the layout of the image
        // Note: If the synchronization2 feature is enabled, when the old and new layout are equal,
        // the layout values are ignored - data is preserved no matter what values are specified,
        // or what layout the image is currently in.
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        
        imageMemoryBarrier.image = image;

        imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, //srcStageMask
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, //dstStageMask
            VK_FLAGS_NONE, //VkDependencyFlags: none = normal barrier, entire image
            0, nullptr, //memory Barriers
            0, nullptr, //buffer Memory Barrier
            1, &imageMemoryBarrier);
    }


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

        PrepareDescriptorSets();
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

    void SimpleComputeShader::SetTexture(int kernelIndex, const std::string& nameID, ImageInfo imageInfo)
    {
        auto it = std::find_if(_bindingInfos.begin(), _bindingInfos.end(), [&nameID](const BindingInfo& item)
            {
                return item.name == nameID;
            });

        BindingInfo& bindingInfo = *it;
        bindingInfo.resourceInfo = imageInfo;

        //TODO : Log error here.
        std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
            vks::initializers::writeDescriptorSet(_descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, bindingInfo.bindingIndex, &imageInfo.descriptor)
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

    

    void SimpleComputeShader::DispatchAllKernels_Test(VkCommandBuffer commandBuffer, int frameIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ, bool imageBarrier)
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

        if (imageBarrier)
        {
            auto it = std::find_if(_bindingInfos.begin(), _bindingInfos.end(), [](const BindingInfo& item)
                {
                    return item.name == "thresholdResult";
                });
            ImageBarrier(commandBuffer, it->resourceInfo.image);
        }

        SetInt("kernelIndex", 1);
        vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, GetTotalSize(_uniformInfos), &_uniformData[0]);

        vkCmdDispatch(commandBuffer, threadGroupsX, threadGroupsY, threadGroupsZ);

        if (imageBarrier)
        {
            auto it = std::find_if(_bindingInfos.begin(), _bindingInfos.end(), [](const BindingInfo& item)
                {
                    return item.name == "blurResult";
                });
            ImageBarrier(commandBuffer, it->resourceInfo.image);
        }

        SetInt("kernelIndex", 2);
        vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, GetTotalSize(_uniformInfos), &_uniformData[0]);

        vkCmdDispatch(commandBuffer, threadGroupsX, threadGroupsY, threadGroupsZ);
    }
}