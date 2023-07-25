#include "SimpleComputeShader.h"
#include "vulkanexamplebase.h"
#include "AssertMsg.h"

#include <regex>
#include <vector>
#include <iostream>

namespace VulkanUtilities
{
    void ValidateUniformAlignment(const std::vector<UniformInfo>& uniforms)
    {
        //This is only a partial validation.
        //Further readind needed:
        // - https://www.reddit.com/r/vulkan/comments/wc0428/on_shader_memory_layout/
        // - https://www.oreilly.com/library/view/opengl-programming-guide/9780132748445/app09lev1sec3.html

        for (int i = 0; i < uniforms.size(); ++i)
        {
            ASSERT_MSG(uniforms[i].type != VulkanUtilities::UniformType::FLOAT2 || uniforms[i].byteOffset % VulkanUtilities::GetTypeSize(uniforms[i].type) == 0,
                       "The alignment of vectors must be equal to 2 or 4 times their base size: 2 when they have 2 rows, and 4 when they have 3 or 4 rows.");

            /*ASSERT_MSG(uniforms[i].type != VulkanUtilities::UniformType::FLOAT4 || uniforms[i].byteOffset % VulkanUtilities::GetTypeSize(uniforms[i].type) == 0,
                "The alignment of vectors must be equal to 2 or 4 times their base size: 2 when they have 2 rows, and 4 when they have 3 or 4 rows.");*/

            //About vec3: https://www.reddit.com/r/vulkan/comments/8vh07n/strange_behavior_with_push_constants/
            /*ASSERT_MSG(uniforms[i].type != VulkanUtilities::UniformType::FLOAT3,
                         "General suggestion: don't use vec3.");*/
        }
    }

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
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,//VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, //srcStageMask
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,//VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, //dstStageMask
            VK_FLAGS_NONE, //VkDependencyFlags: none = normal barrier, entire image
            0, nullptr, //memory Barriers
            0, nullptr, //buffer Memory Barrier
            1, &imageMemoryBarrier);
    }


    std::vector<VkDescriptorSetLayoutBinding> getDescriptorSetLayout(const std::vector<BindingInfo>& bindingInfos)
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

        //This below is for Advec comp shader:
        /*
        setLayoutBindings.push_back(VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT });
        setLayoutBindings.push_back(VkDescriptorSetLayoutBinding{ 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT });
        setLayoutBindings.push_back(VkDescriptorSetLayoutBinding{ 2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT });
        */
        
        for (auto bindingInfo : bindingInfos)
        {
            setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(bindingInfo.type, VK_SHADER_STAGE_COMPUTE_BIT, bindingInfo.bindingIndex));
        }

        return setLayoutBindings;
    }

    SimpleComputeShader::SimpleComputeShader(VulkanFramework& framework, const std::string& shaderAssetPath)
        : _framework(framework)
    {
        ASSERT_MSG(0, "Shader Parsing was not yet fully implemented.");

        _uniformInfos = ParseShaderUniforms(shaderAssetPath);
        _shaderAssetPath = shaderAssetPath;
        int totalSize = GetTotalSize(_uniformInfos);
        if (totalSize > 0)
        {
            _uniformData.resize(totalSize, (unsigned char)0);
        }
        
        _bindingInfos = ParseShaderBindings(_shaderAssetPath);

        PrepareDescriptorSets();
    }

    SimpleComputeShader::SimpleComputeShader(VulkanFramework& framework, const std::string& shaderAssetPath, const std::vector<UniformInfo>& uniforms, const std::vector<BindingInfo>& bindings)
        : _framework(framework)
    {
        _uniformInfos = uniforms;
        ValidateUniformAlignment(_uniformInfos);
        _shaderAssetPath = shaderAssetPath;
        int totalSize = GetTotalSize(_uniformInfos);
        if (totalSize > 0)
        {
            _uniformData.resize(totalSize, (unsigned char)0);
        }

        _bindingInfos = bindings;

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
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = getDescriptorSetLayout(_bindingInfos);

        VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(_framework.getVkDevice(), &descriptorLayout, nullptr, &_descriptorSetLayout));

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(&_descriptorSetLayout, 1);
        
        int pushConstantsSize = GetTotalSize(_uniformInfos);
        if (pushConstantsSize > 0)
        {
            _pushConstantRange.offset = 0;
            _pushConstantRange.size = GetTotalSize(_uniformInfos);
            _pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
            pPipelineLayoutCreateInfo.pPushConstantRanges = &_pushConstantRange;
        }

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

        std::string fileName = _framework.getShadersPath() + _shaderAssetPath + ".comp.spv";
        computePipelineCreateInfo.stage = _framework.loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);

        VK_CHECK_RESULT(vkCreateComputePipelines(_framework.getVkDevice(), _framework.getPipelineCache(), 1, &computePipelineCreateInfo, nullptr, &_pipeline));
    }

    void SimpleComputeShader::SetFloat(const std::string& name, float val)
    {
        SetValue(_uniformInfos, _uniformData, name, &val);
    }

    void SimpleComputeShader::SetFloat2(const std::string& name, float valx, float valy)
    {
        glm::vec2 val = { valx, valy };
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

        ASSERT_MSG(it != _bindingInfos.end(), "SimpleComputeShader::SetTexture() nameID="+ nameID +" not found in binding infos");

        BindingInfo& bindingInfo = *it;
        bindingInfo.resourceInfo = imageInfo;

        //TODO : Log error here.
        std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
            vks::initializers::writeDescriptorSet(_descriptorSet, bindingInfo.type, bindingInfo.bindingIndex, &imageInfo.descriptor)
        };
        vkUpdateDescriptorSets(_framework.getVkDevice(), computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);
    }

    void SimpleComputeShader::UpdateUniforms(VkCommandBuffer commandBuffer)
    {
        if (_uniformInfos.size() > 0)
        {
            vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, GetTotalSize(_uniformInfos), &_uniformData[0]);
        }
    }

    void SimpleComputeShader::Dispatch(VkCommandBuffer commandBuffer, int kernelIndex, int frameIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ)
    {
        //ANSME : Should this be called before or after bind pipeline?
        UpdateUniforms(commandBuffer);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1, &_descriptorSet, 0, 0);
        vkCmdDispatch(commandBuffer, threadGroupsX, threadGroupsY, threadGroupsZ);
    }

    

    void SimpleComputeShader::DispatchAllKernels_Test(VkCommandBuffer commandBuffer, int frameIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ, bool imageBarrier)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1, &_descriptorSet, 0, 0);
        
        SetFloat("alpha", 0.663);
        SetInt("frameIndex", frameIndex);
        SetInt("kernelIndex", 0);
        UpdateUniforms(commandBuffer);
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
        UpdateUniforms(commandBuffer);
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
        UpdateUniforms(commandBuffer);

        vkCmdDispatch(commandBuffer, threadGroupsX, threadGroupsY, threadGroupsZ);
    }
}