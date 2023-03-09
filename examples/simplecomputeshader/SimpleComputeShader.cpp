#include "SimpleComputeShader.h"
#include "vulkanexamplebase.h"

#include <regex>
#include <vector>
#include <iostream>

namespace VulkanUtilities
{
    UniformType GetUniformType(std::string type)
    {
        if (type == "float")
        {
            return UniformType::FLOAT;
        }
        else if (type == "int")
        {
            return UniformType::INT;
        }
        return UNSUPPORTED;
    }

    int GetTypeSize(UniformType type)
    {
        switch (type)
        {
        case UniformType::FLOAT: return sizeof(float);
        case UniformType::INT: return sizeof(int);
        default: return 0;
        }
    }

    int GetTotalSize(std::vector<UniformInfo>& uniforms)
    {
        return uniforms.back().byteOffset + GetTypeSize(uniforms.back().type);
    }

    void SetValue(const std::vector<UniformInfo>& uniformInfos, std::vector<unsigned char>& uniformData, std::string name, void* src)
    {
        for (int i = 0; i < uniformInfos.size(); ++i)
        {
            if (uniformInfos[i].name == name)
            {
                int offset = uniformInfos[i].byteOffset;
                memcpy(&uniformData[offset], src, GetTypeSize(uniformInfos[i].type));
                return;
            }
        }
    }
    
    std::vector<UniformInfo> GetGlobalVariables()
    {
        std::string code = R"(
            int kernelIndex;
            int frameIndex;
            float alpha;

        )";

        std::string code1 = R"(
        // StableFluids - A GPU implementation of Jos Stam's Stable Fluids on Unity
        // https://github.com/keijiro/StableFluids

        #pragma kernel Advect
        #pragma kernel Force
        #pragma kernel PSetup
        #pragma kernel PFinish
        #pragma kernel Jacobi1
        #pragma kernel Jacobi2
        #pragma kernel Vorticity1
        #pragma kernel Vorticity2


        // Common parameter
        int TestBegin;
        float Time;
        float DeltaTime;

        float Vorticity;

        // F (external forces)
        Texture2D<float2> F_in;

        // U (velocity field)
        Texture2D<float2> U_in;
        SamplerState samplerU_in;
        RWTexture2D<float2> U_out;

        // W (velocity field; working)
        Texture2D<float2> W_in;
        RWTexture2D<float2> W_out;

        // Div W
        RWTexture2D<float> DivW_out;

        // P (pressure field)
        Texture2D<float> P_in;
        RWTexture2D<float> P_out;

        // Color map
        Texture2D<half4> C_in;
        SamplerState samplerC_in;
        RWTexture2D<half4> C_out;

        // Obstacles map
        Texture2D<float> O_in;
        SamplerState samplerO_in;

        // Jacobi method arguments
        float Alpha;
        float Beta;
        int TestEnd;

        Texture2D<float> X1_in;
        Texture2D<float> B1_in;
        RWTexture2D<float> X1_out;

        Texture2D<float2> X2_in;
        Texture2D<float2> B2_in;
        RWTexture2D<float2> X2_out;

        Texture2D<float> VOR_in;
        RWTexture2D<float> VOR_out;

        [numthreads(8, 8, 1)]
        void Vorticity2(uint2 tid : SV_DispatchThreadID)
        {
        }"; )";

        std::vector<UniformInfo> globalVars;

        // Regular expression to match global variable declarations
        //std::regex regex("(?:^|\\n)\\s*(?:extern\\s+)?([a-zA-Z_][a-zA-Z0-9_]*)\\s+([a-zA-Z_][a-zA-Z0-9_<>\\[\\]\\s*]*)\\s*(?:=\\s*[^;]+)?;", std::regex::multiline);
        std::regex regex("(?:^|\\n)\\s*(?:extern\\s+)?([a-zA-Z_][a-zA-Z0-9_<>]*)\\s+([a-zA-Z_][a-zA-Z0-9_\\[\\]\\s*]*)\\s*(?:=\\s*[^;]+)?;");
        /*
        The regular expression used to match global variable declarations is (?:^|\\n)\\s*(?:extern\\s+)?([a-zA-Z_][a-zA-Z0-9_]*)\\s+([a-zA-Z_][a-zA-Z0-9_<>\\[\\]\\s*]*)\\s*(?:=\\s*[^;]+)?;. Here's a breakdown of the different parts:

(?:^|\\n) matches the beginning of the line or a newline character
\\s* matches any number of whitespace characters
(?:extern\\s+)? optionally matches the keyword "extern" followed by whitespace
([a-zA-Z_][a-zA-Z0-9_]*) matches the variable name, which must start with a letter or underscore and can be followed by any combination of letters, underscores, and digits
\\s+ matches one or more whitespace characters
([a-zA-Z_][a-zA-Z0-9_<>\\[\\]\\s*]*) matches the variable type, which can include templates, arrays, and pointers, as well as whitespace
\\s*(?:=\\s*[^;]+)? optionally matches an initializer expression, which is any sequence of characters that doesn't contain a semicolon, preceded by an equals sign and any amount of whitespace
;
*/
        // Iterate over all matches
        std::sregex_iterator it(code.begin(), code.end(), regex);
        std::sregex_iterator end;
        int order = 0;
        int byteOffset = 0;
        while (it != end) {
            std::smatch match = *it;
            std::string varType = match[1].str();
            std::string varName = match[2].str();

            if (varType.find("Texture2D") == std::string::npos &&
                varType.find("SamplerState") == std::string::npos)
            {
                UniformType uType = GetUniformType(varType);
                int typeSize = GetTypeSize(uType);
                globalVars.push_back(UniformInfo{ varName, uType, order, byteOffset });
                byteOffset += typeSize;
                ++order;
            }

            ++it;
        }

        return globalVars;
    }

    void PushBindind(std::vector<BindingInfo>& bindingInfos, const std::string& name, VkDescriptorType type, VkFormat format)
    {
        bindingInfos.push_back(BindingInfo{ name, type, format, (uint32_t)bindingInfos.size() });
    }

    std::vector<BindingInfo> readBindingInfosFromExampleShaders(const std::string& shader)
    {
        std::vector<BindingInfo> bindingInfos;
        PushBindind(bindingInfos, "inputImage", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        PushBindind(bindingInfos, "resultImage", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        return bindingInfos;
    }

    std::vector<BindingInfo> readBindingInfosFromMultiKernelTestShader(const std::string& shader)
    {
        std::vector<BindingInfo> bindingInfos;
        PushBindind(bindingInfos, "thresholdInput", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        PushBindind(bindingInfos, "thresholdResult", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        PushBindind(bindingInfos, "blurInput", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        PushBindind(bindingInfos, "blurResult", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        PushBindind(bindingInfos, "channelSwapInput", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        PushBindind(bindingInfos, "channelSwapResult", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        return bindingInfos;
    }

    std::vector<BindingInfo> readBindingInfosFromUnityShader(const std::string& shader)
    {
        std::vector<BindingInfo> bindingInfos;
        
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
        _uniformInfos = GetGlobalVariables();
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
        //_bindingInfos = readBindingInfosFromExampleShaders(_shaderName);
        _bindingInfos = readBindingInfosFromMultiKernelTestShader(_shaderName);

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


