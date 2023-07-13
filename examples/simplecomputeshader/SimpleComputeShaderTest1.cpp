/*
* Vulkan Example - Compute shader image processing
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "SimpleComputeShaderTest1.h"
#include "VulkanFramework.h"
#include "SimpleComputeShader.h"

namespace
{
    struct Vertex {
        float pos[3];
        float uv[2];
    };
}

SimpleComputeShaderTest1::SimpleComputeShaderTest1() : VulkanExampleBase(ENABLE_VALIDATION)
{
    title = "Simple Compute shader test 1";
    camera.type = Camera::CameraType::lookat;
    camera.setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
    camera.setRotation(glm::vec3(0.0f));
    camera.setPerspective(60.0f, (float)width * 0.5f / (float)height, 1.0f, 256.0f);
}

SimpleComputeShaderTest1::~SimpleComputeShaderTest1()
{
    // Graphics
    vkDestroyPipeline(device, graphics.pipeline, nullptr);
    vkDestroyPipelineLayout(device, graphics.pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, graphics.descriptorSetLayout, nullptr);
    vkDestroySemaphore(device, graphics.semaphore, nullptr);

    // Compute
    for (auto& computePass : compute.passes)
    {
        delete computePass.computeShader;
        computePass.computeShader = nullptr;
    }

    vkDestroySemaphore(device, compute.semaphore, nullptr);
    vkDestroyCommandPool(device, compute.commandPool, nullptr);

    vertexBuffer.destroy();
    indexBuffer.destroy();
    uniformBufferVS.destroy();

    textureColorMap.destroy();

    for (auto& computePass : compute.passes)
    {
        computePass.textureComputeTarget.destroy();
    }
}

vks::Texture2D& SimpleComputeShaderTest1::lastTextureComputeTarget()
{
    size_t lastComputePassIdx = compute.passes.size() - 1;
    return compute.passes[lastComputePassIdx].textureComputeTarget;
}

void SimpleComputeShaderTest1::loadAssets()
{
    textureColorMap.loadFromFile(getAssetPath() + "textures/vulkan_11_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
}

void SimpleComputeShaderTest1::createComputePasses()
{
    //framework = new VulkanUtilities::VulkanExampleFramework(*this, descriptorPool, pipelineCache);// (*this, descriptorPool, pipelineCache);

    std::vector<std::string> shaderNames = { "simplecomputeshader/threshold", "simplecomputeshader/blur", "simplecomputeshader/channelswap", "simplecomputeshader/threshold" };

    for (auto shaderName : shaderNames)
    {
        ComputePass computePass;
        computePass.shaderName = shaderName;
        compute.passes.push_back(computePass);
    }
}

// Setup vertices for a single uv-mapped quad
void SimpleComputeShaderTest1::generateQuad()
{
    // Setup vertices for a single uv-mapped quad made from two triangles
    std::vector<Vertex> vertices =
    {
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f } },
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f } },
        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
        { {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } }
    };

    // Setup indices
    std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
    indexCount = static_cast<uint32_t>(indices.size());

    // Create buffers
    // For the sake of simplicity we won't stage the vertex data to the gpu memory
    // Vertex buffer
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &vertexBuffer,
        vertices.size() * sizeof(Vertex),
        vertices.data()));
    // Index buffer
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &indexBuffer,
        indices.size() * sizeof(uint32_t),
        indices.data()));
}

void SimpleComputeShaderTest1::setupVertexDescriptions()
{
    // Binding description
    vertices.bindingDescriptions = {
        vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
    };

    // Attribute descriptions
    // Describes memory layout and shader positions
    vertices.attributeDescriptions = {
        // Location 0: Position
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)),
        // Location 1: Texture coordinates
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)),
    };

    // Assign to vertex buffer
    vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertices.inputState.vertexBindingDescriptionCount = vertices.bindingDescriptions.size();
    vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
    vertices.inputState.vertexAttributeDescriptionCount = vertices.attributeDescriptions.size();
    vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

// Prepare and initialize uniform buffer containing shader uniforms
void SimpleComputeShaderTest1::prepareUniformBuffers()
{
    // Vertex shader uniform buffer block
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &uniformBufferVS,
        sizeof(uboVS)));

    // Map persistent
    VK_CHECK_RESULT(uniformBufferVS.map());

    updateUniformBuffers();
}

void SimpleComputeShaderTest1::updateUniformBuffers()
{
    uboVS.projection = camera.matrices.perspective;
    uboVS.modelView = camera.matrices.view;
    memcpy(uniformBufferVS.mapped, &uboVS, sizeof(uboVS));
}

void SimpleComputeShaderTest1::prepareTextureTarget(vks::Texture* tex, uint32_t width, uint32_t height, VkFormat format)
{
    VkFormatProperties formatProperties;

    // Get device properties for the requested texture format
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
    // Check if requested image format supports image storage operations
    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

    // Prepare blit target texture
    tex->width = width;
    tex->height = height;

    VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = { width, height, 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Image will be sampled in the fragment shader and used as storage target in the compute shader
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageCreateInfo.flags = 0;
    // If compute and graphics queue family indices differ, we create an image that can be shared between them
    // This can result in worse performance than exclusive sharing mode, but save some synchronization to keep the sample simple
    std::vector<uint32_t> queueFamilyIndices;
    if (vulkanDevice->queueFamilyIndices.graphics != vulkanDevice->queueFamilyIndices.compute) {
        queueFamilyIndices = {
            vulkanDevice->queueFamilyIndices.graphics,
            vulkanDevice->queueFamilyIndices.compute
        };
        imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        imageCreateInfo.queueFamilyIndexCount = 2;
        imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }

    VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs;

    VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &tex->image));

    vkGetImageMemoryRequirements(device, tex->image, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &tex->deviceMemory));
    VK_CHECK_RESULT(vkBindImageMemory(device, tex->image, tex->deviceMemory, 0));

    VkCommandBuffer layoutCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    tex->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    vks::tools::setImageLayout(
        layoutCmd, tex->image,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        tex->imageLayout);

    vulkanDevice->flushCommandBuffer(layoutCmd, queue, true);

    // Create sampler
    VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler.addressModeV = sampler.addressModeU;
    sampler.addressModeW = sampler.addressModeU;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.compareOp = VK_COMPARE_OP_NEVER;
    sampler.minLod = 0.0f;
    sampler.maxLod = tex->mipLevels;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &tex->sampler));

    // Create image view
    VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
    view.image = VK_NULL_HANDLE;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = format;
    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    view.image = tex->image;
    VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &tex->view));

    // Initialize a descriptor for later use
    tex->descriptor.imageLayout = tex->imageLayout;
    tex->descriptor.imageView = tex->view;
    tex->descriptor.sampler = tex->sampler;
    tex->device = vulkanDevice;
}

void SimpleComputeShaderTest1::setupGraphicsDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0: Vertex shader uniform buffer
        vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
        // Binding 1: Fragment shader input image
        vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &graphics.descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&graphics.descriptorSetLayout, 1);
    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &graphics.pipelineLayout));
}

void SimpleComputeShaderTest1::preparePipelines()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
        vks::initializers::pipelineInputAssemblyStateCreateInfo(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            0,
            VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState =
        vks::initializers::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_NONE,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
            0);

    VkPipelineColorBlendAttachmentState blendAttachmentState =
        vks::initializers::pipelineColorBlendAttachmentState(
            0xf,
            VK_FALSE);

    VkPipelineColorBlendStateCreateInfo colorBlendState =
        vks::initializers::pipelineColorBlendStateCreateInfo(
            1,
            &blendAttachmentState);

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
        vks::initializers::pipelineDepthStencilStateCreateInfo(
            VK_TRUE,
            VK_TRUE,
            VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineViewportStateCreateInfo viewportState =
        vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

    VkPipelineMultisampleStateCreateInfo multisampleState =
        vks::initializers::pipelineMultisampleStateCreateInfo(
            VK_SAMPLE_COUNT_1_BIT,
            0);

    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState =
        vks::initializers::pipelineDynamicStateCreateInfo(
            dynamicStateEnables.data(),
            dynamicStateEnables.size(),
            0);

    // Rendering pipeline
    // Load shaders
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    shaderStages[0] = loadShader(getShadersPath() + "computeshadernetwork/texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader(getShadersPath() + "computeshadernetwork/texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo =
        vks::initializers::pipelineCreateInfo(
            graphics.pipelineLayout,
            renderPass,
            0);

    pipelineCreateInfo.pVertexInputState = &vertices.inputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    pipelineCreateInfo.renderPass = renderPass;

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &graphics.pipeline));
}

void SimpleComputeShaderTest1::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        // Graphics pipelines uniform buffers
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
        // Graphics pipelines image samplers for displaying compute output image
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
    };

    for (auto& computePass : compute.passes)
    {
        poolSizes.push_back(
            // Compute pipelines uses a storage image for image reads and writes
            vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2)
        );
    }

    VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, poolSizes.size());
    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void SimpleComputeShaderTest1::setupDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(descriptorPool, &graphics.descriptorSetLayout, 1);

    // Input image (before compute post processing)
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &graphics.descriptorSetPreCompute));
    std::vector<VkWriteDescriptorSet> baseImageWriteDescriptorSets = {
        vks::initializers::writeDescriptorSet(graphics.descriptorSetPreCompute, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBufferVS.descriptor),
        vks::initializers::writeDescriptorSet(graphics.descriptorSetPreCompute, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textureColorMap.descriptor)
    };
    vkUpdateDescriptorSets(device, baseImageWriteDescriptorSets.size(), baseImageWriteDescriptorSets.data(), 0, nullptr);

    // Final image (after compute shader processing)
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &graphics.descriptorSetPostCompute));
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        vks::initializers::writeDescriptorSet(graphics.descriptorSetPostCompute, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBufferVS.descriptor),
        vks::initializers::writeDescriptorSet(graphics.descriptorSetPostCompute, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &lastTextureComputeTarget().descriptor)
    };
    vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void SimpleComputeShaderTest1::prepareGraphics()
{
    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &graphics.semaphore));

    // Signal the semaphore
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &graphics.semaphore;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));
}

void SimpleComputeShaderTest1::prepareCompute()
{
    //testUnityCompute();
    framework = new VulkanUtilities::VulkanExampleFramework(*this, descriptorPool, pipelineCache);// (*this, descriptorPool, pipelineCache);

    // Get a compute queue from the device
    vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.compute, 0, &compute.queue);

    vks::Texture* srcImage = &textureColorMap;
    for (auto& computePass : compute.passes)
    {
        computePass.computeShader = new VulkanUtilities::SimpleComputeShader(*framework, computePass.shaderName);
        computePass.computeShader->SetTexture(0, "inputImage", *srcImage);
        computePass.computeShader->SetTexture(0, "resultImage", computePass.textureComputeTarget);

        //Input texture is output of the previous compute pass
        srcImage = &computePass.textureComputeTarget;
    }

    // One pipeline for each effect
    for (auto& computePass : compute.passes)
    {
        //TODO : Test pipeline creation in SimpleComputeShader constructor
        computePass.computeShader->CreatePipeline();
    }

    // Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = vulkanDevice->queueFamilyIndices.compute;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &compute.commandPool));

    // Create a command buffer for compute operations
    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        vks::initializers::commandBufferAllocateInfo(
            compute.commandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1);

    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &compute.commandBuffer));

    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &compute.semaphore));

    // Build a single command buffer containing the compute dispatch commands
    buildComputeCommandBuffer();
}

void SimpleComputeShaderTest1::buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VkClearValue clearValues[2];
    clearValues[0].color = defaultClearColor;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

        // Image memory barrier to make sure that compute shader writes are finished before sampling from the texture
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        // We won't be changing the layout of the image
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.image = lastTextureComputeTarget().image;
        imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        vkCmdPipelineBarrier(
            drawCmdBuffers[i],
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_FLAGS_NONE,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = vks::initializers::viewport((float)width * 0.5f, (float)height, 0.0f, 1.0f);
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

        VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
        vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        // Left (pre compute)
        vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSetPreCompute, 0, NULL);
        vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline);

        vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);

        // Right (post compute)
        vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSetPostCompute, 0, NULL);
        vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline);

        viewport.x = (float)width / 2.0f;
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
        vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);

        drawUI(drawCmdBuffers[i]);

        vkCmdEndRenderPass(drawCmdBuffers[i]);

        {
            VkClearColorValue clearColor0 = { 1.0f, 0.0f, 0.0f, 1.0f };

            VkImageSubresourceRange clearRange;
            clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            clearRange.baseMipLevel = 0;
            clearRange.levelCount = VK_REMAINING_MIP_LEVELS;
            clearRange.baseArrayLayer = 0;
            clearRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
            vkCmdClearColorImage(drawCmdBuffers[i], compute.passes[0].textureComputeTarget.image, VK_IMAGE_LAYOUT_GENERAL, &clearColor0, 1, &clearRange);
        }

        VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
    }

}

void SimpleComputeShaderTest1::buildComputeCommandBuffer()
{
    // Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
    vkQueueWaitIdle(compute.queue);

    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VK_CHECK_RESULT(vkBeginCommandBuffer(compute.commandBuffer, &cmdBufInfo));

    for (auto& computePass : compute.passes)
    {
        computePass.computeShader->Dispatch(compute.commandBuffer, 0, 0, computePass.textureComputeTarget.width / 16, computePass.textureComputeTarget.height / 16, 1);
    }
    
    vkEndCommandBuffer(compute.commandBuffer);
}

void SimpleComputeShaderTest1::draw()
{
    // Wait for rendering finished
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    // Submit compute commands
    VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
    computeSubmitInfo.commandBufferCount = 1;
    computeSubmitInfo.pCommandBuffers = &compute.commandBuffer;
    computeSubmitInfo.waitSemaphoreCount = 1;
    computeSubmitInfo.pWaitSemaphores = &graphics.semaphore;
    computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
    computeSubmitInfo.signalSemaphoreCount = 1;
    computeSubmitInfo.pSignalSemaphores = &compute.semaphore;
    VK_CHECK_RESULT(vkQueueSubmit(compute.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));	
    VulkanExampleBase::prepareFrame();

    VkPipelineStageFlags graphicsWaitStageMasks[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::vector<VkSemaphore> graphicsWaitSemaphores = { compute.semaphore, semaphores.presentComplete };
    std::vector<VkSemaphore> graphicsWaitSemaphoresTest = { semaphores.presentComplete };
    VkSemaphore graphicsSignalSemaphores[] = { graphics.semaphore, semaphores.renderComplete };

    // Submit graphics commands
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
    submitInfo.waitSemaphoreCount = computeSemaphore ? graphicsWaitSemaphores.size() : graphicsWaitSemaphoresTest.size();
    submitInfo.pWaitSemaphores = computeSemaphore ? &graphicsWaitSemaphores[0] : &graphicsWaitSemaphoresTest[0];
    submitInfo.pWaitDstStageMask = graphicsWaitStageMasks;
    submitInfo.signalSemaphoreCount = 2;
    submitInfo.pSignalSemaphores = graphicsSignalSemaphores;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

    VulkanExampleBase::submitFrame();
}

void SimpleComputeShaderTest1::render()
{
    if (!prepared)
        return;
    draw();
    if (camera.updated) {
        updateUniformBuffers();
    }
}

void SimpleComputeShaderTest1::viewChanged()
{
    camera.setPerspective(60.0f, (float)width * 0.5f / (float)height, 1.0f, 256.0f);
    updateUniformBuffers();
}

void SimpleComputeShaderTest1::OnUpdateUIOverlay(vks::UIOverlay *overlay)
{
    if (overlay->header("Settings")) {
        /*if (overlay->comboBox("Shader", &compute.pipelineIndex, shaderNames)) {
            buildComputeCommandBuffer();
        }*/
        overlay->checkBox("Semaphore", &computeSemaphore);
        overlay->checkBox("ImageBarrier", &imageBarrier);
    }
}
