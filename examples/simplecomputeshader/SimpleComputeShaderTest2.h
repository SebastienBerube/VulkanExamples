#ifndef SIMPLE_COMPUTE_SHADER_TEST2_H_
#define SIMPLE_COMPUTE_SHADER_TEST2_H_

/*
* Vulkan Example - Compute shader image processing
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "vulkanexamplebase.h"
#include "SimpleComputeShader.h"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false

// Vertex layout for this example
/*
*/

class SimpleComputeShaderTest2 : public VulkanExampleBase
{
private:
    vks::Texture2D textureColorMap;
public:
    struct {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;

    // Resources for the graphics part of the example
    struct {
        VkDescriptorSetLayout descriptorSetLayout;  // Image display shader binding layout
        VkDescriptorSet descriptorSetPreCompute;    // Image display shader bindings before compute shader image manipulation
        VkDescriptorSet descriptorSetPostCompute;   // Image display shader bindings after compute shader image manipulation
        VkPipeline pipeline;                        // Image display pipeline
        VkPipelineLayout pipelineLayout;            // Layout of the graphics pipeline
        VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
    } graphics;

    struct ComputePass {
        VulkanUtilities::SimpleComputeShader* computeShader;
        std::string shaderName;
        vks::Texture2D thresholdResult;
        vks::Texture2D blurResult;
        vks::Texture2D channelSwapResult;
    };

    // Resources for the compute part of the example
    struct Compute {
        VkQueue queue;                                // Separate queue for compute commands (queue family may differ from the one used for graphics)
        VkCommandPool commandPool;                    // Use a separate command pool (queue family may differ from the one used for graphics)
        VkCommandBuffer commandBuffer;                // Command buffer storing the dispatch commands and barriers
        VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
        ComputePass pass;
    } compute;

    VulkanUtilities::VulkanExampleFramework* framework;

    vks::Buffer vertexBuffer;
    vks::Buffer indexBuffer;
    uint32_t indexCount;
    bool computeSemaphore = true;

    vks::Buffer uniformBufferVS;

    struct {
        glm::mat4 projection;
        glm::mat4 modelView;
    } uboVS;

    int vertexBufferSize;

    SimpleComputeShaderTest2();

    ~SimpleComputeShaderTest2();

    vks::Texture2D& lastTextureComputeTarget();

    void loadAssets();

    void createComputePasses();

    void generateQuad();

    void setupVertexDescriptions();

    void prepareUniformBuffers();

    void updateUniformBuffers();

    void prepareTextureTarget(vks::Texture* tex, uint32_t width, uint32_t height, VkFormat format);

    void setupGraphicsDescriptorSetLayout();

    void preparePipelines();

    void setupDescriptorPool();

    void setupDescriptorSet();

    void prepareGraphics();

    void testUnityCompute();

    void prepareCompute();

    void buildCommandBuffers();

    void buildComputeCommandBuffer();

    void draw();

    void prepare()
    {
        VulkanExampleBase::prepare();
        loadAssets();
        createComputePasses();
        generateQuad();
        setupVertexDescriptions();
        prepareUniformBuffers();
        
        //for(auto& computePass : compute.passes)
        {
            prepareTextureTarget(&compute.pass.thresholdResult, textureColorMap.width, textureColorMap.height, VK_FORMAT_R8G8B8A8_UNORM);
            prepareTextureTarget(&compute.pass.blurResult, textureColorMap.width, textureColorMap.height, VK_FORMAT_R8G8B8A8_UNORM);
            prepareTextureTarget(&compute.pass.channelSwapResult, textureColorMap.width, textureColorMap.height, VK_FORMAT_R8G8B8A8_UNORM);
            
        }
        
        setupGraphicsDescriptorSetLayout();
        preparePipelines();
        setupDescriptorPool();
        setupDescriptorSet();
        prepareGraphics();
        prepareCompute();
        buildCommandBuffers();
        prepared = true;
    }

    virtual void render();

    virtual void viewChanged();

    virtual void OnUpdateUIOverlay(vks::UIOverlay* overlay);
};

#endif //SIMPLE_COMPUTE_SHADER_TEST2_H_