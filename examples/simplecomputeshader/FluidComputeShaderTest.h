#ifndef FLUID_COMPUTE_SHADER_TEST1_H_
#define FLUID_COMPUTE_SHADER_TEST1_H_

/*
* Vulkan Example - Compute shader image processing
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "vulkanexamplebase.h"
#include "SimpleComputeShader.h"
#include "AssertMsg.h"
#include <map>

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false

class FluidComputeShaderTest : public VulkanExampleBase
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

    enum class eComputePass
    {
        Advect,
        ForceGen,
        Force,
        PSetup,
        PFinish,
        Jacobi1A,
        Jacobi1B,
        Jacobi2,
        Vorticity1,
        Vorticity2
    };

    enum class eTexID : int
    {
        V1,
        V2,
        V3,
        V4,
        F1,
        D1,
        P1,
        P2,
        VOR
    };

    struct ComputePass {
        ComputePass(eComputePass id, std::string shaderName);
        VulkanUtilities::SimpleComputeShader* computeShader;
        eComputePass id;
        std::string shaderName;
    };

    std::map<eTexID, vks::Texture2D> computeTextureTargets;

    // Resources for the compute part of the example
    struct Compute {
        VkQueue queue;                                // Separate queue for compute commands (queue family may differ from the one used for graphics)
        VkCommandPool commandPool;                    // Use a separate command pool (queue family may differ from the one used for graphics)
        VkCommandBuffer commandBuffer;                // Command buffer storing the dispatch commands and barriers
        VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
        std::vector<ComputePass> passes;
        int threadGroupX, threadGroupY;
        int computeResX, computeResY;
    } compute;

    VulkanUtilities::VulkanExampleFramework* framework;

    vks::Buffer vertexBuffer;
    vks::Buffer indexBuffer;
    uint32_t indexCount;
    bool computeSemaphore = true;
    bool imageBarrier = true;

    vks::Buffer uniformBufferVS;

    struct {
        glm::mat4 projection;
        glm::mat4 modelView;
    } uboVS;

    int frameNo = 0;
    float totalTimeSec = 0.0f;
    glm::vec2 previousInputPos;

    int vertexBufferSize;

    FluidComputeShaderTest();

    ~FluidComputeShaderTest();

    vks::Texture2D& lastTextureComputeTarget();

    void loadAssets();

    void createComputeTextureTarget(FluidComputeShaderTest::eTexID id, VkFormat format);

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
    
    ComputePass* getComputePassById(eComputePass id);

    void prepareCompute();

    void buildCommandBuffers();

    void buildComputeCommandBuffer();

    void updateComputeShaderPushConstants();

    void draw();

    void prepare()
    {
        frameNo = 0;
        VulkanExampleBase::prepare();
        loadAssets();
        createComputePasses();
        generateQuad();
        setupVertexDescriptions();
        prepareUniformBuffers();
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

#endif //FLUID_COMPUTE_SHADER_TEST1_H_