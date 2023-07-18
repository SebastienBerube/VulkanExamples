/*
 * Shader related util functions
 *
 * Copyright (C) 2023 by Sebastien Berube
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "VulkanShaders.h"

#include <dxcapi.h>
#include <fstream>
#include <vector>
#include <iostream>

#include <wrl/client.h>
using namespace Microsoft::WRL;

/*#if defined(__ANDROID__)
#define VK_CHECK_RESULT(f)																				\
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        LOGE("Fatal : VkResult is \" %s \" in %s at line %d", vks::tools::errorString(res).c_str(), __FILE__, __LINE__); \
        assert(res == VK_SUCCESS);																		\
    }																									\
}
#else
#define VK_CHECK_RESULT(f)																				\
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        std::cout << "Fatal : VkResult is \"" << vks::tools::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
        assert(res == VK_SUCCESS);																		\
    }																									\
}
#endif*/

namespace vks
{
    namespace shaders
    {
#ifndef __ANDROID__

        std::string getCompilationErrors(ComPtr<IDxcResult>& result)
        {
            std::string errors;
            ComPtr<IDxcBlobWide> outputName = {};
            ComPtr<IDxcBlobUtf8> dxcErrorInfo = {};
            result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&dxcErrorInfo), &outputName);

            if (dxcErrorInfo != nullptr) {
                errors = std::string(dxcErrorInfo->GetStringPointer());
            }

            return errors;
        }

        std::string loadTextFile(const std::string& filePath)
        {
            std::ifstream file(filePath);
            std::string content;

            if (file.is_open()) {
                // Read the entire file into a string
                content.assign((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
                file.close();
            }
            else {
                std::cerr << "Failed to open file." << std::endl;
            }
            return content;
        }

        ComPtr<IDxcResult> compileHLSL(IDxcCompiler3* dxc_compiler, const std::string& hlslText, std::vector<LPCWSTR>& args)
        {
            ComPtr<IDxcResult> result;

            DxcBuffer src_buffer;
            src_buffer.Ptr = &*hlslText.begin();
            src_buffer.Size = hlslText.size();
            src_buffer.Encoding = 0;

            dxc_compiler->Compile(&src_buffer, args.data(), args.size(), nullptr, IID_PPV_ARGS(&result));

            return result;
        }

        void printErrors(std::string& errors)
        {
            if (!errors.empty())
            {
                std::cerr << "Errors : \n" << errors << std::endl;
            }
        }

        LPCWSTR getDxcShaderStageArg(VkShaderStageFlagBits shaderStage)
        {
            /*
                https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-models

                See also: https ://github.com/SaschaWillems/Vulkan/commit/73cbc2900a1eb533da2e34171071a63d1b761635
                Commit: 
                    Sascha Willems 26 Mar 2023 04:19 : 29 - 04 : 00
                    Compile GLSL and HLSL shaders using CMake
                    Work - in - progress
                    
                cmake sample below:

                    if (${ FILE_EXT } STREQUAL ".vert")
                    set(PROFILE "vs_6_1")
                    elseif(${ FILE_EXT } STREQUAL ".frag")
                    set(PROFILE "ps_6_1")
                    elseif(${ FILE_EXT } STREQUAL ".comp")
                    set(PROFILE "cs_6_1")
                    elseif(${ FILE_EXT } STREQUAL ".geom")
                    set(PROFILE "gs_6_1")
                    elseif(${ FILE_EXT } STREQUAL ".tesc")
                    set(PROFILE "hs_6_1")
                    elseif(${ FILE_EXT } STREQUAL ".tese")
                    set(PROFILE "ds_6_1")
                    # elseif(${ FILE_EXT } STREQUAL ".rgen")
                    # @todo
                    # set(PROFILE "lib_6_3")
                    endif()
                    # message(${ DXC_BINARY } - spirv - T ${ PROFILE } - E main ${ HLSL } - Fo ${ SPIRV })
                    set(SPIRV "${SHADER_DIR_HLSL}/${FILE_NAME}.spv")
                    add_custom_command(OUTPUT ${ SPIRV } COMMAND ${ DXC_BINARY } - spirv - T ${ PROFILE } - E main ${ HLSL } - Fo ${ SPIRV } DEPENDS ${ HLSL })
                    list(APPEND SPIRV_BINARY_FILES ${ SPIRV })
                    endforeach(HLSL)
            */

            switch (shaderStage)
            {
            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return L"hs_6_0";
            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return L"ds_6_0";
            case VK_SHADER_STAGE_VERTEX_BIT: return L"vs_6_0";
            case VK_SHADER_STAGE_FRAGMENT_BIT: return L"ps_6_0";
            case VK_SHADER_STAGE_GEOMETRY_BIT: return L"gs_6_0";
            case VK_SHADER_STAGE_COMPUTE_BIT: return L"cs_6_0";
            default: return L"cs_6_0";
            }
        }

        ComPtr<IDxcBlob> compileHlslInternal(const char* fileName, VkDevice device, VkShaderStageFlagBits shaderStage)
        {
            //OPTME : instance of ComPtr<IDxcCompiler3> should be kept and reused.
            ComPtr<IDxcUtils> dxc_utils = {};
            ComPtr<IDxcCompiler3> dxc_compiler = {};
            DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils));
            DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler)); \

            std::vector<LPCWSTR> args;
            args.push_back(L"-Zpc");
            args.push_back(L"-HV");
            args.push_back(L"2021");
            args.push_back(L"-T");
            args.push_back(getDxcShaderStageArg(shaderStage));
            args.push_back(L"-E");
            args.push_back(L"main");
            args.push_back(L"-spirv");
            args.push_back(L"-fspv-target-env=vulkan1.1");

            ComPtr<IDxcResult> result = compileHLSL(*dxc_compiler.GetAddressOf(), loadTextFile(fileName), args);
            std::string errors = getCompilationErrors(result);
            if (!errors.empty())
            {
                printErrors(errors);
                return VK_NULL_HANDLE;
            }

            HRESULT status = 0;
            HRESULT hr = result->GetStatus(&status);
            if (FAILED(hr) || FAILED(status))
            {
                throw std::runtime_error("IDxcResult::GetStatus failed with HRESULT = " + status);
            }

            ComPtr<IDxcBlob> shader_obj;
            ComPtr<IDxcBlobWide> outputName = {};
            hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader_obj), &outputName);
            if (FAILED(hr) || FAILED(status))
            {
                throw std::runtime_error("IDxcResult::GetStatus failed with HRESULT = " + status);
            }

            const auto shader_size = shader_obj->GetBufferSize();
            if (shader_size % sizeof(std::uint32_t) != 0)
            {
                throw std::runtime_error("Invalid SPIR-V buffer size");
            }

            return shader_obj;
        }

        void compileHlsl(const char* fileName, VkDevice device, VkShaderStageFlagBits shaderStage, size_t& shaderCodeSize, uint32_t*& shaderCode)
        {
            auto shaderObj = compileHlslInternal(fileName, device, shaderStage);

            shaderCodeSize = shaderObj->GetBufferSize();

            const size_t ELEM_SIZE = sizeof(std::uint32_t);

            if (shaderCodeSize % ELEM_SIZE != 0)
            {
                throw std::runtime_error("Invalid buffer size: size in bytes should be a multiple of sizeof(uint32_t)");
            }

            shaderCode = new uint32_t[shaderCodeSize/ELEM_SIZE];

            //memcpy_s((char*)shaderCode, shaderCodeSize, shaderObj->GetBufferPointer(), shaderCodeSize);
            std::memcpy(shaderCode, shaderObj->GetBufferPointer(), shaderCodeSize);
        }

        VkShaderModule compileAndLoadHlslShader(const char* fileName, VkDevice device, VkShaderStageFlagBits shaderStage)
        {
            auto shader_obj = compileHlslInternal(fileName, device, shaderStage);

            VkShaderModule shaderModule;
            VkShaderModuleCreateInfo moduleCreateInfo{};
            moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleCreateInfo.codeSize = shader_obj->GetBufferSize();
            moduleCreateInfo.pCode = (uint32_t*)shader_obj->GetBufferPointer();

            VkResult res = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule);

            //VK_CHECK_RESULT(res);

            return shaderModule;
        }
#endif

        VkShaderModule loadShaderFromSource(const char* fileName, VkDevice device, ShadingLanguage shadingLang, VkShaderStageFlagBits shaderStage)
        {
            switch (shadingLang) {
            case ShadingLanguage::HLSL:
                return compileAndLoadHlslShader(fileName, device, shaderStage);
            default:
                std::cerr << "Error: runtime shader compilation is only supported for HLSL\n";
                return VK_NULL_HANDLE;
            }
        }

        bool fileExists(const std::string &filename)
        {
            std::ifstream f(filename.c_str());
            return !f.fail();
        }
    }
}
