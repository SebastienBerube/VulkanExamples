/*
* Vulkan Example - Using subpasses for G-Buffer compositing
*
* Copyright (C) 2016-2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*
* Summary:
* Implements a deferred rendering setup with a forward transparency pass using sub passes
*
* Sub passes allow reading from the previous framebuffer (in the same render pass) at
* the same pixel position.
*
* This is a feature that was especially designed for tile-based-renderers
* (mostly mobile GPUs) and is a new optimization feature in Vulkan for those GPU types.
*
*/

#include "vulkanexamplebase.h"
#include "VulkanglTFModel.h"

#define ENABLE_VALIDATION false

#define NUM_LIGHTS 64

class VulkanExample : public VulkanExampleBase
{
public:
	struct {
		vks::Texture2D glass;
	} textures;

	struct {
		vkglTF::Model scene;
		vkglTF::Model transparent;
	} models;

	struct {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
	} uboGBuffer;

	struct Light {
		glm::vec4 position;
		glm::vec3 color;
		float radius;
	};

	struct {
		glm::vec4 viewPos;
		Light lights[NUM_LIGHTS];
	} uboLights;

	struct {
		vks::Buffer GBuffer;
		vks::Buffer lights;
	} uniformBuffers;

	struct {
		VkPipeline offscreen;
		VkPipeline composition;
		VkPipeline transparent;
	} pipelines;

	struct {
		VkPipelineLayout offscreen;
		VkPipelineLayout composition;
		VkPipelineLayout transparent;
	} pipelineLayouts;

	struct {
		VkDescriptorSet scene;
		VkDescriptorSet composition;
		VkDescriptorSet transparent;
	} descriptorSets;

	struct {
		VkDescriptorSetLayout scene;
		VkDescriptorSetLayout composition;
		VkDescriptorSetLayout transparent;
	} descriptorSetLayouts;

	// G-Buffer framebuffer attachments
	struct FrameBufferAttachment {
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory mem = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkFormat format;
	};
	struct Attachments {
		FrameBufferAttachment position, normal, albedo;
		int32_t width;
		int32_t height;
	} attachments;

    VulkanExample();

    ~VulkanExample();

	// Enable physical device features required for this example
    virtual void getEnabledFeatures();;

    void clearAttachment(FrameBufferAttachment* attachment);

	// Create a frame buffer attachment
    void createAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment* attachment);

	// Create color attachments for the G-Buffer components
    void createGBufferAttachments();

	// Override framebuffer setup from base class, will automatically be called upon setup and if a window is resized
    void setupFrameBuffer();

	// Override render pass setup from base class
    void setupRenderPass();

    void buildCommandBuffers();

    void loadAssets();

    void setupDescriptorPool();

    void setupDescriptorSetLayout();

    void setupDescriptorSet();

    void preparePipelines();

	// Create the Vulkan objects used in the composition pass (descriptor sets, pipelines, etc.)
    void prepareCompositionPass();

	// Prepare and initialize uniform buffer containing shader uniforms
    void prepareUniformBuffers();

    void updateUniformBufferDeferredMatrices();

    void initLights();

	// Update fragment shader light position uniform block
    void updateUniformBufferDeferredLights();

    void draw();

    void prepare();

    virtual void render();

    virtual void viewChanged();

    virtual void OnUpdateUIOverlay(vks::UIOverlay* overlay);
};

//VULKAN_EXAMPLE_MAIN()
