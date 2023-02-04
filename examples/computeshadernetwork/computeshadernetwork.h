/*
* Vulkan Example - Compute shader image processing
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "vulkanexamplebase.h"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false

// Vertex layout for this example
struct Vertex {
    float pos[3];
    float uv[2];
};

class VulkanExample : public VulkanExampleBase
{
private:
    vks::Texture2D textureColorMap;
    vks::Texture2D textureComputeTarget;
public:
    struct {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;

    // Resources for the graphics part of the example
    struct {
        VkDescriptorSetLayout descriptorSetLayout;	// Image display shader binding layout
        VkDescriptorSet descriptorSetPreCompute;	// Image display shader bindings before compute shader image manipulation
        VkDescriptorSet descriptorSetPostCompute;	// Image display shader bindings after compute shader image manipulation
        VkPipeline pipeline;						// Image display pipeline
        VkPipelineLayout pipelineLayout;			// Layout of the graphics pipeline
        VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
    } graphics;

    // Resources for the compute part of the example
    struct Compute {
        VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
        VkCommandPool commandPool;					// Use a separate command pool (queue family may differ from the one used for graphics)
        VkCommandBuffer commandBuffer;				// Command buffer storing the dispatch commands and barriers
        VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
        VkDescriptorSetLayout descriptorSetLayout;	// Compute shader binding layout
        VkDescriptorSet descriptorSet;				// Compute shader bindings
        VkPipelineLayout pipelineLayout;			// Layout of the compute pipeline
        std::vector<VkPipeline> pipelines;			// Compute pipelines for image filters
        int32_t pipelineIndex = 0;					// Current image filtering compute pipeline index
    } compute;

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

    std::vector<std::string> shaderNames;

    VulkanExample();

    ~VulkanExample();

    void prepareTextureTarget(vks::Texture* tex, uint32_t width, uint32_t height, VkFormat format);

    void loadAssets();

    void buildCommandBuffers();

    void buildComputeCommandBuffer();

    void generateQuad();

    void setupVertexDescriptions();

    void setupDescriptorPool();

    void setupDescriptorSetLayout();

    void setupDescriptorSet();

    void preparePipelines();

    void prepareGraphics();

    void prepareCompute();

    void prepareUniformBuffers();

    void updateUniformBuffers();

    void draw();

    void prepare();

    virtual void render();

    virtual void viewChanged();

    virtual void OnUpdateUIOverlay(vks::UIOverlay* overlay);
};
