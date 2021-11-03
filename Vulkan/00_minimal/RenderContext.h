#pragma once

struct VulkanRenderContext
{
	static const int PENDING_FRAMES = 2;

	VulkanDeviceContext* context;

	uint32_t graphicsQueueIndex;
	uint32_t presentQueueIndex;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkCommandPool mainCommandPool;
	VkCommandBuffer mainCommandBuffers[PENDING_FRAMES];
	VkFence mainFences[PENDING_FRAMES];
	VkFramebuffer framebuffers[PENDING_FRAMES];
	VkRenderPass renderPass;

	VkImageSubresourceRange mainSubRange;

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	VkPipeline graphicsPipeline;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;


	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<VkBuffer> uniformBufferSurfaceModel;
	std::vector<VkDeviceMemory> uniformBufferSurfaceModelMemory;

	VkDescriptorPool descriptorPool;

	uint32_t mipLevels;
	VkImage textureImage;
	VkImageView textureImageView;
    VkDeviceMemory textureImageMemory;
	VkSampler textureSampler;

	VkImage textureNormalImage;
	VkImageView textureNormalImageView;
    VkDeviceMemory textureNormalImageMemory;
	VkSampler textureNormalSampler;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	//MSAA
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;
};
