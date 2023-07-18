/*
 * Shader related util functions
 *
 * Copyright (C) 2023 by Sebastien Berube
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#pragma once

#include "vulkan/vulkan.h"
#include "VulkanInitializers.hpp"

enum class ShadingLanguage {
    HLSL = 0x00000001,
    GLSL = 0x00000002,
};

namespace vks
{
    namespace shaders
    {
        void compileHlsl(const char* fileName, VkDevice device, VkShaderStageFlagBits shaderStage, size_t& shaderCodeSize, uint32_t*& shaderCode);

        VkShaderModule loadShaderFromSource(const char* fileName, VkDevice device, ShadingLanguage shadingLang, VkShaderStageFlagBits shaderStage);
    }
}
