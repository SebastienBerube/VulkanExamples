#include <regex>
#include <iostream>
#include "ShaderParser.h"
#include "AssertMsg.h"

namespace VulkanUtilities
{
    UniformType GetUniformType(const std::string& type)
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
        return uniforms.size() > 0 ? uniforms.back().byteOffset + GetTypeSize(uniforms.back().type)
                                   : 0;
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
        ASSERT_MSG(0, "Uniform does not exist: " + name + ". Have you forgotten to push a valid UniformInfo?");
    }

    std::vector<UniformInfo> ParseShaderUniforms(const std::string& shader)
    {
        //Temporary, until a better parser is used to perform the actual task.
        std::string testCode;

        if (shader.find("multikerneltest") != std::string::npos)
        {
            testCode = R"(
                int kernelIndex;
                int frameIndex;
                float alpha;

            )";
        }
        if (shader.find("Advect") != std::string::npos)
        {
            testCode = R"(
                float DeltaTime;
            )";
        }
        else if (shader.find("FluidSimCommon.compute") != std::string::npos)
        {
            testCode = R"(
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
        }
        else
        {
            testCode = R"(
                #version 450

                layout (local_size_x = 16, local_size_y = 16) in;
                layout (binding = 0, rgba8) uniform readonly image2D inputImage;
                layout (binding = 1, rgba8) uniform image2D resultImage;
            )";
        }

        std::vector<UniformInfo> globalVars;

        // Regular expression to match global variable declarations
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
        */
        // Iterate over all matches
        std::sregex_iterator it(testCode.begin(), testCode.end(), regex);
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

    std::vector<BindingInfo> ReadBindingInfosFromAdvectShader(const std::string& shader)
    {
        std::vector<BindingInfo> bindingInfos;
        PushBindind(bindingInfos, "samplerU_in", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_FORMAT_R32G32_SFLOAT);
        PushBindind(bindingInfos, "U_in", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32G32_SFLOAT);
        PushBindind(bindingInfos, "W_out", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R32G32_SFLOAT);
        return bindingInfos;
    }

    std::vector<BindingInfo> ReadBindingInfosFromExampleShaders(const std::string& shader)
    {
        std::vector<BindingInfo> bindingInfos;
        PushBindind(bindingInfos, "inputImage", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        PushBindind(bindingInfos, "resultImage", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_FORMAT_R8G8B8A8_UNORM);
        return bindingInfos;
    }

    std::vector<BindingInfo> ReadBindingInfosFromMultiKernelTestShader(const std::string& shader)
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

    std::vector<BindingInfo> ReadBindingInfosFromUnityShader(const std::string& shader)
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

    std::vector<BindingInfo> ParseShaderBindings(const std::string& shader)
    {
        if (shader.find("multikerneltest") != std::string::npos)
        {
            return ReadBindingInfosFromMultiKernelTestShader(shader);
        }
        else if (shader.find("FluidSimCommon.compute") != std::string::npos)
        {
            return ReadBindingInfosFromUnityShader(shader);
        }
        else if (shader.find("Advect") != std::string::npos)
        {
            return ReadBindingInfosFromAdvectShader(shader);
        }
        
        return ReadBindingInfosFromExampleShaders(shader);
    }
}