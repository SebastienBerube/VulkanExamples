/*
 * Assorted commonly used Vulkan helper functions
 *
 * Copyright (C) 2016-2023 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "VulkanTools.h"

#include <dxcapi.h>

#include <wrl/client.h>
using namespace Microsoft::WRL;

#if !(defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
// iOS & macOS: VulkanExampleBase::getAssetPath() implemented externally to allow access to Objective-C components
const std::string getAssetPath()
{
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	return "";
#elif defined(VK_EXAMPLE_ASSETS_DIR)
	return VK_EXAMPLE_ASSETS_DIR;
#else
	return "./../assets/";
#endif
}
#endif

#if !(defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
// iOS & macOS: VulkanExampleBase::getAssetPath() implemented externally to allow access to Objective-C components
const std::string getShaderBasePath()
{
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	return "shaders/";
#elif defined(VK_EXAMPLE_SHADERS_DIR)
	return VK_EXAMPLE_SHADERS_DIR;
#else
	return "./../shaders/";
#endif
}
#endif

namespace vks
{
	namespace tools
	{
		bool errorModeSilent = false;

		std::string errorString(VkResult errorCode)
		{
			switch (errorCode)
			{
#define STR(r) case VK_ ##r: return #r
				STR(NOT_READY);
				STR(TIMEOUT);
				STR(EVENT_SET);
				STR(EVENT_RESET);
				STR(INCOMPLETE);
				STR(ERROR_OUT_OF_HOST_MEMORY);
				STR(ERROR_OUT_OF_DEVICE_MEMORY);
				STR(ERROR_INITIALIZATION_FAILED);
				STR(ERROR_DEVICE_LOST);
				STR(ERROR_MEMORY_MAP_FAILED);
				STR(ERROR_LAYER_NOT_PRESENT);
				STR(ERROR_EXTENSION_NOT_PRESENT);
				STR(ERROR_FEATURE_NOT_PRESENT);
				STR(ERROR_INCOMPATIBLE_DRIVER);
				STR(ERROR_TOO_MANY_OBJECTS);
				STR(ERROR_FORMAT_NOT_SUPPORTED);
				STR(ERROR_SURFACE_LOST_KHR);
				STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
				STR(SUBOPTIMAL_KHR);
				STR(ERROR_OUT_OF_DATE_KHR);
				STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
				STR(ERROR_VALIDATION_FAILED_EXT);
				STR(ERROR_INVALID_SHADER_NV);
				STR(ERROR_INCOMPATIBLE_SHADER_BINARY_EXT);
#undef STR
			default:
				return "UNKNOWN_ERROR";
			}
		}

		std::string physicalDeviceTypeString(VkPhysicalDeviceType type)
		{
			switch (type)
			{
#define STR(r) case VK_PHYSICAL_DEVICE_TYPE_ ##r: return #r
				STR(OTHER);
				STR(INTEGRATED_GPU);
				STR(DISCRETE_GPU);
				STR(VIRTUAL_GPU);
				STR(CPU);
#undef STR
			default: return "UNKNOWN_DEVICE_TYPE";
			}
		}

		VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat)
		{
			// Since all depth formats may be optional, we need to find a suitable depth format to use
			// Start with the highest precision packed format
			std::vector<VkFormat> formatList = {
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM
			};

			for (auto& format : formatList)
			{
				VkFormatProperties formatProps;
				vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
				if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					*depthFormat = format;
					return true;
				}
			}

			return false;
		}

		VkBool32 getSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat)
		{
			std::vector<VkFormat> formatList = {
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
			};

			for (auto& format : formatList)
			{
				VkFormatProperties formatProps;
				vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
				if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					*depthStencilFormat = format;
					return true;
				}
			}

			return false;
		}


		VkBool32 formatHasStencil(VkFormat format)
		{
			std::vector<VkFormat> stencilFormats = {
				VK_FORMAT_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};
			return std::find(stencilFormats.begin(), stencilFormats.end(), format) != std::end(stencilFormats);
		}

		// Returns if a given format support LINEAR filtering
		VkBool32 formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

			if (tiling == VK_IMAGE_TILING_OPTIMAL)
				return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

			if (tiling == VK_IMAGE_TILING_LINEAR)
				return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

			return false;
		}

		// Create an image memory barrier for changing the layout of
		// an image and put it into an active command buffer
		// See chapter 11.4 "Image Layout" for details

		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask)
		{
			// Create an image barrier object
			VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			// Source layouts (old)
			// Source access mask controls actions that have to be finished on the old layout
			// before it will be transitioned to the new layout
			switch (oldImageLayout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				// Image layout is undefined (or does not matter)
				// Only valid as initial layout
				// No flags required, listed only for completeness
				imageMemoryBarrier.srcAccessMask = 0;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				// Image is preinitialized
				// Only valid as initial layout for linear images, preserves memory contents
				// Make sure host writes have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image is a color attachment
				// Make sure any writes to the color buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image is a depth/stencil attachment
				// Make sure any writes to the depth/stencil buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image is a transfer source
				// Make sure any reads from the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image is a transfer destination
				// Make sure any writes to the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image is read by a shader
				// Make sure any shader reads from the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
			}

			// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
			switch (newImageLayout)
			{
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image will be used as a transfer destination
				// Make sure any writes to the image have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image will be used as a transfer source
				// Make sure any reads from the image have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image will be used as a color attachment
				// Make sure any writes to the color buffer have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image layout will be used as a depth/stencil attachment
				// Make sure any writes to depth/stencil buffer have been finished
				imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image will be read in a shader (sampler, input attachment)
				// Make sure any writes to the image have been finished
				if (imageMemoryBarrier.srcAccessMask == 0)
				{
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				}
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
			}

			// Put barrier inside setup command buffer
			vkCmdPipelineBarrier(
				cmdbuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}

		// Fixed sub resource on first mip level and layer
		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageAspectFlags aspectMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask)
		{
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = aspectMask;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.layerCount = 1;
			setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
		}

		void insertImageMemoryBarrier(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange)
		{
			VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
			imageMemoryBarrier.srcAccessMask = srcAccessMask;
			imageMemoryBarrier.dstAccessMask = dstAccessMask;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(
				cmdbuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}

		void exitFatal(const std::string& message, int32_t exitCode)
		{
#if defined(_WIN32)
			if (!errorModeSilent) {
				MessageBox(NULL, message.c_str(), NULL, MB_OK | MB_ICONERROR);
			}
#elif defined(__ANDROID__)
			LOGE("Fatal error: %s", message.c_str());
			vks::android::showAlert(message.c_str());
#endif
			std::cerr << message << "\n";
#if !defined(__ANDROID__)
			exit(exitCode);
#endif
		}

		void exitFatal(const std::string& message, VkResult resultCode)
		{
			exitFatal(message, (int32_t)resultCode);
		}

#if defined(__ANDROID__)
		// Android shaders are stored as assets in the apk
		// So they need to be loaded via the asset manager
		VkShaderModule loadShader(AAssetManager* assetManager, const char *fileName, VkDevice device, ShaderFileType type)
        {
            assert(type == ShaderFileType::SPV);

            if (type != ShaderFileType::SPV)
            {
                std::cerr << "On Android, only spv shaders can be loaded." << std::endl;
            }

			// Load shader from compressed asset
			AAsset* asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_STREAMING);
			assert(asset);
			size_t size = AAsset_getLength(asset);
			assert(size > 0);

			char *shaderCode = new char[size];
			AAsset_read(asset, shaderCode, size);
			AAsset_close(asset);

			VkShaderModule shaderModule;
			VkShaderModuleCreateInfo moduleCreateInfo;
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.pNext = NULL;
			moduleCreateInfo.codeSize = size;
			moduleCreateInfo.pCode = (uint32_t*)shaderCode;
			moduleCreateInfo.flags = 0;

			VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

			delete[] shaderCode;

			return shaderModule;
		}
#else

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

			VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

			return shaderModule;
		}

        VkShaderModule loadShader(const char* fileName, VkDevice device)
        {
            std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

            if (is.is_open())
            {
                size_t size = is.tellg();
                is.seekg(0, std::ios::beg);
                char* shaderCode = new char[size];
                is.read(shaderCode, size);
                is.close();

                assert(size > 0);

                VkShaderModule shaderModule;
                VkShaderModuleCreateInfo moduleCreateInfo{};
                moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                moduleCreateInfo.codeSize = size;
                moduleCreateInfo.pCode = (uint32_t*)shaderCode;

                VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

                delete[] shaderCode;

                return shaderModule;
            }
            else
            {
                std::cerr << "Error: Could not open shader file \"" << fileName << "\"" << "\n";
                return VK_NULL_HANDLE;
            }
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

		uint32_t alignedSize(uint32_t value, uint32_t alignment)
        {
	        return (value + alignment - 1) & ~(alignment - 1);
        }

		size_t alignedSize(size_t value, size_t alignment)
		{
			return (value + alignment - 1) & ~(alignment - 1);
		}

	}
}
