#pragma once

#include "vk_common.h"
#include "DeviceContext.h"
#include "RenderContext.h"


struct GLFWwindow;

struct VulkanGraphicsApplication
{
	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices 
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() 
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	const char* name;
	VulkanDeviceContext context;
	VulkanRenderContext rendercontext;
	GLFWwindow* window;

	//

	uint32_t m_imageIndex;
	uint32_t m_frame;
	uint32_t m_currentFrame;

	bool Initialize(const char *);
	bool Prepare();
	bool Run();
	bool Update();
	bool Begin();
	bool End();
	bool Display();
	void Terminate();
	bool Shutdown();

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	void CreateSwapChain();

	void CreateRenderPath();
	void CreateGraphicsPipeline();
	void CreateFrameBuffer();
	void CreateVertexBuffer();
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CreateIndexBuffer();
	void CreateCommandPool();
	void CreateCommandBuffer();
	void CreateDescriptorSetLayout();
	void CreateUniformBuffer();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void CreateTextureImage();
	void CreateTextureNormalImage();
	void CreateTextureImageView();
	void CreateTextureNormalImageView();
	void CreateImageViews();
	void CreateTextureSampler();
	void CreateTextureNormalSampler();
	//void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	void CreateDepthResources();
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();

	VkSampleCountFlagBits GetMaxUsableSampleCount();
	void CreateColorResources();

	void LoadModel();

	void UpdateUniformBuffer(uint32_t currentImage, float totalTime);
	void UpdateSurfaceModel(uint32_t currentImage, float totalTime);

};
