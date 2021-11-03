
#include "vk_common.h"

#include "DeviceContext.h"
#include "RenderContext.h"
#include "GraphicsApplication.h"

//#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "../OBJ/viking_room.obj";
const std::string TEXTURE_PATH = "../textures/dash-lined-metal_albedo.png";
const std::string TEXTURE_NORMAL_PATH = "../textures/dash-lined-metal_normal-ogl.png";

#include "Mesh.h"
#include "Vertex.h"
#include "SurfaceModel.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm/gtx/hash.hpp>

#define APP_NAME "00_minimal"


/*
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;


	static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

	    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};
*/

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };

}

//const std::vector<Vertex> vertices = {
//    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
//
//    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
//};
//
//const std::vector<uint16_t> indices = {
//    0, 1, 2, 2, 3, 0,
//    4, 5, 6, 6, 7, 4
//};

struct UniformBufferObject {
   alignas(16) glm::mat4 model;
   alignas(16) glm::mat4 view;
   alignas(16) glm::mat4 proj;
};



	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

Mesh mesh{};

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

//uint32_t findMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
//	VkPhysicalDeviceMemoryProperties memProperties;
//	vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
//
//	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
//		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
//		    return i;
//		}
//	}
//
//throw std::runtime_error("failed to find suitable memory type!");
//}

//
// Initialisation des ressources 
//

void VulkanGraphicsApplication::CreateFrameBuffer()
{
	// create frame buffer
	for (size_t i = 0; i < rendercontext.PENDING_FRAMES; i++)
	{
		std::array<VkImageView, 2> attachments = 
		{
			context.swapchainImageViews[i],
			rendercontext.depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = rendercontext.renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = context.swapchainExtent.width;
		framebufferInfo.height = context.swapchainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(context.device, &framebufferInfo, nullptr, &(rendercontext.framebuffers[i])) != VK_SUCCESS) 
		{
		  throw std::runtime_error("failed to create framebuffer!");
		}
	}

}

void VulkanGraphicsApplication::CreateGraphicsPipeline()
{
	// load shader
	auto vertShaderCode = readFile("vert.spv");
	auto fragShaderCode = readFile("frag.spv");

	rendercontext.vertShaderModule = context.createShaderModule(vertShaderCode);
	rendercontext.fragShaderModule = context.createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = rendercontext.vertShaderModule;
	vertShaderStageInfo.pName = "main";
	
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = rendercontext.fragShaderModule;
	fragShaderStageInfo.pName = "main";
	
	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
	
	// Vertex bind description
	// Vertex attribute description

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	//pipeline vertex input
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	


	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) context.swapchainExtent.width;
	viewport.height = (float) context.swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = context.swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// multi sampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	//depth stencil
	 VkPipelineDepthStencilStateCreateInfo depthStencil{};
     depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
     depthStencil.depthTestEnable = VK_TRUE;
     depthStencil.depthWriteEnable = VK_TRUE;
     depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
     depthStencil.depthBoundsTestEnable = VK_FALSE;
     depthStencil.stencilTestEnable = VK_FALSE;
	
	//color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional
	
	//dynamic state
	VkDynamicState dynamicStates[] = {
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_LINE_WIDTH
	};
	
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
	
	//pipeline layout
	//VkPipelineLayout pipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &(rendercontext.descriptorSetLayout);
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	
	if (vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr, &rendercontext.pipelineLayout) != VK_SUCCESS) {
	    throw std::runtime_error("failed to create pipeline layout!");
	}


	//pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = rendercontext.pipelineLayout;
	pipelineInfo.renderPass = rendercontext.renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &rendercontext.graphicsPipeline) != VK_SUCCESS) {
	    throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(context.device, rendercontext.vertShaderModule, nullptr);
    vkDestroyShaderModule(context.device, rendercontext.fragShaderModule, nullptr);
}

void VulkanGraphicsApplication::CreateVertexBuffer()
{
	//VkPhysicalDeviceMemoryProperties memProperties;
	//vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &memProperties);
	
	VkDeviceSize bufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();

	VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	// filling the vertex buffer

	void* data;
	vkMapMemory(context.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mesh.vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(context.device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.vertexBuffer, mesh.vertexBufferMemory);

	CopyBuffer(stagingBuffer, mesh.vertexBuffer, bufferSize);

	vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingBufferMemory, nullptr);
}

void VulkanGraphicsApplication::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = rendercontext.mainCommandPool;
        allocInfo.commandBufferCount = 1;

		VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &memProperties);

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(rendercontext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(rendercontext.graphicsQueue);

        vkFreeCommandBuffers(context.device, rendercontext.mainCommandPool, 1, &commandBuffer);
}

void VulkanGraphicsApplication:: CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	// buffer creation
	
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
    if (vkCreateBuffer(context.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

	// memory requirement

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(context.device, buffer, &memRequirements);

	// memory allocation

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = context.findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(context.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(context.device, buffer, bufferMemory, 0);

}

void VulkanGraphicsApplication::CreateIndexBuffer()
{
	   VkDeviceSize bufferSize = sizeof(mesh.indices[0]) * mesh.indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(context.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mesh.indices.data(), (size_t) bufferSize);
    vkUnmapMemory(context.device, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.indexBuffer, mesh.indexBufferMemory);

    CopyBuffer(stagingBuffer, mesh.indexBuffer, bufferSize);

    vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingBufferMemory, nullptr);
}

void VulkanGraphicsApplication::CreateRenderPath()
{
	// create render pass
	VkAttachmentDescription colorAttachment{};
    colorAttachment.format = context.surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(context.device, &renderPassInfo, nullptr, &rendercontext.renderPass) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanGraphicsApplication::CreateCommandPool()
{
	//init command pool
	VkCommandPoolCreateInfo poolInfo;
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = rendercontext.graphicsQueueIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.pNext = nullptr;

	if (vkCreateCommandPool(context.device, &poolInfo, nullptr, &rendercontext.mainCommandPool) != VK_SUCCESS)
	{
			throw std::runtime_error("failed to create command pool!");
	}
}

void VulkanGraphicsApplication::CreateCommandBuffer()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = rendercontext.mainCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) 2; // = PENDING_FRAMES



	if (vkAllocateCommandBuffers(context.device, &allocInfo, rendercontext.mainCommandBuffers) != VK_SUCCESS) 
	{
	    throw std::runtime_error("failed to allocate command buffers!");
	}
}

void VulkanGraphicsApplication::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		
		VkDescriptorSetLayoutBinding samplerNormalLayoutBinding{};
        samplerNormalLayoutBinding.binding = 2;
        samplerNormalLayoutBinding.descriptorCount = 1;
        samplerNormalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerNormalLayoutBinding.pImmutableSamplers = nullptr;
        samplerNormalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 3> bindings = {uboLayoutBinding, samplerLayoutBinding, samplerNormalLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();//&uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &rendercontext.descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
}

void VulkanGraphicsApplication::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    rendercontext.uniformBuffers.resize(VulkanDeviceContext::SWAPCHAIN_IMAGES);
    rendercontext.uniformBuffersMemory.resize(VulkanDeviceContext::SWAPCHAIN_IMAGES);

    for (size_t i = 0; i < VulkanDeviceContext::SWAPCHAIN_IMAGES; i++) {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, rendercontext.uniformBuffers[i], rendercontext.uniformBuffersMemory[i]);
    }
}

void VulkanGraphicsApplication::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 3> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(VulkanDeviceContext::SWAPCHAIN_IMAGES);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(VulkanDeviceContext::SWAPCHAIN_IMAGES);
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[2].descriptorCount = static_cast<uint32_t>(VulkanDeviceContext::SWAPCHAIN_IMAGES);

	       // VkDescriptorPoolSize poolSize{};
        //poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //poolSize.descriptorCount = static_cast<uint32_t>(VulkanDeviceContext::SWAPCHAIN_IMAGES);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(VulkanDeviceContext::SWAPCHAIN_IMAGES);

        if (vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &rendercontext.descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
}

void VulkanGraphicsApplication::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(VulkanDeviceContext::SWAPCHAIN_IMAGES, rendercontext.descriptorSetLayout);
       VkDescriptorSetAllocateInfo allocInfo{};
       allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
       allocInfo.descriptorPool = rendercontext.descriptorPool;
       allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanDeviceContext::SWAPCHAIN_IMAGES);
       allocInfo.pSetLayouts = layouts.data();

       rendercontext.descriptorSets.resize(VulkanDeviceContext::SWAPCHAIN_IMAGES);
       if (vkAllocateDescriptorSets(context.device, &allocInfo, rendercontext.descriptorSets.data()) != VK_SUCCESS) {
           throw std::runtime_error("failed to allocate descriptor sets!");
       }

       for (size_t i = 0; i < VulkanDeviceContext::SWAPCHAIN_IMAGES; i++) {
           VkDescriptorBufferInfo bufferInfo{};
           bufferInfo.buffer = rendercontext.uniformBuffers[i];
           bufferInfo.offset = 0;
           bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = rendercontext.textureImageView;
		imageInfo.sampler = rendercontext.textureSampler;

		VkDescriptorImageInfo imagenormalInfo{};
		imagenormalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imagenormalInfo.imageView = rendercontext.textureNormalImageView;
		imagenormalInfo.sampler = rendercontext.textureNormalSampler;

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = rendercontext.descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = rendercontext.descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = rendercontext.descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &imagenormalInfo;

		vkUpdateDescriptorSets(context.device, 3, descriptorWrites.data(), 0, nullptr);
	}
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) 
{
			VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	  
		VkShaderModule shaderModule;
if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
}

return shaderModule;
}

void VulkanGraphicsApplication::CreateDepthResources() 
{
        VkFormat depthFormat = FindDepthFormat();

        CreateImage(context.swapchainExtent.width, context.swapchainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, rendercontext.depthImage, rendercontext.depthImageMemory);
        rendercontext.depthImageView = CreateImageView(rendercontext.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

		TransitionImageLayout(rendercontext.depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

	 VkFormat VulkanGraphicsApplication::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
	 {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(context.physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

VkFormat VulkanGraphicsApplication::FindDepthFormat() 
{
	return FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
}

bool hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanGraphicsApplication::UpdateUniformBuffer(uint32_t currentImage, float totalTime)
{
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), totalTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), context.swapchainExtent.width / (float) context.swapchainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        void* data;
        vkMapMemory(context.device, rendercontext.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(context.device, rendercontext.uniformBuffersMemory[currentImage]);
}

void VulkanGraphicsApplication::UpdateSurfaceModel(uint32_t currentImage, float totalTime)
{

}

void VulkanGraphicsApplication::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) 
{
	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(context.physicalDevice, imageFormat, &formatProperties);
	
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) 
	{
	    throw std::runtime_error("texture image format does not support linear blitting!");
	}
	
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;
	
	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;
	
	for (uint32_t i = 1; i < mipLevels; i++) {
	    barrier.subresourceRange.baseMipLevel = i - 1;
	    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	
	    vkCmdPipelineBarrier(commandBuffer,
	        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
	        0, nullptr,
	        0, nullptr,
	        1, &barrier);
	
	    VkImageBlit blit{};
	    blit.srcOffsets[0] = {0, 0, 0};
	    blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
	    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	    blit.srcSubresource.mipLevel = i - 1;
	    blit.srcSubresource.baseArrayLayer = 0;
	    blit.srcSubresource.layerCount = 1;
	    blit.dstOffsets[0] = {0, 0, 0};
	    blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
	    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	    blit.dstSubresource.mipLevel = i;
	    blit.dstSubresource.baseArrayLayer = 0;
	    blit.dstSubresource.layerCount = 1;
	
	    vkCmdBlitImage(commandBuffer,
	        image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	        image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	        1, &blit,
	        VK_FILTER_LINEAR);
	
	    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	
	    vkCmdPipelineBarrier(commandBuffer,
	        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
	        0, nullptr,
	        0, nullptr,
	        1, &barrier);
	
	    if (mipWidth > 1) mipWidth /= 2;
	    if (mipHeight > 1) mipHeight /= 2;
	}
	
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	
	vkCmdPipelineBarrier(commandBuffer,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
	    0, nullptr,
	    0, nullptr,
	    1, &barrier);

    EndSingleTimeCommands(commandBuffer);
}

void VulkanGraphicsApplication::CreateTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
	rendercontext.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;


    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

	VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(context.device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(context.device, stagingBufferMemory);

        stbi_image_free(pixels);

        CreateImage(texWidth, texHeight, rendercontext.mipLevels, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, rendercontext.textureImage, rendercontext.textureImageMemory);

        TransitionImageLayout(rendercontext.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, rendercontext.mipLevels);
        CopyBufferToImage(stagingBuffer, rendercontext.textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        TransitionImageLayout(rendercontext.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,  rendercontext.mipLevels);

        vkDestroyBuffer(context.device, stagingBuffer, nullptr);
        vkFreeMemory(context.device, stagingBufferMemory, nullptr);

		GenerateMipmaps(rendercontext.textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, rendercontext.mipLevels);
}

void VulkanGraphicsApplication::CreateTextureNormalImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(TEXTURE_NORMAL_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
	rendercontext.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;


    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

	VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(context.device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(context.device, stagingBufferMemory);

        stbi_image_free(pixels);

        CreateImage(texWidth, texHeight, rendercontext.mipLevels, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, rendercontext.textureImage, rendercontext.textureImageMemory);

        TransitionImageLayout(rendercontext.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, rendercontext.mipLevels);
        CopyBufferToImage(stagingBuffer, rendercontext.textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        TransitionImageLayout(rendercontext.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,  rendercontext.mipLevels);

        vkDestroyBuffer(context.device, stagingBuffer, nullptr);
        vkFreeMemory(context.device, stagingBufferMemory, nullptr);

		GenerateMipmaps(rendercontext.textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, rendercontext.mipLevels);
}

void VulkanGraphicsApplication::CreateTextureImageView() 
{
    rendercontext.textureImageView = CreateImageView(rendercontext.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, rendercontext.mipLevels);
}

void VulkanGraphicsApplication::CreateTextureNormalImageView() 
{
    rendercontext.textureNormalImageView = CreateImageView(rendercontext.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, rendercontext.mipLevels);
}

void VulkanGraphicsApplication::CreateImageViews() {
    //context.swapchainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < VulkanDeviceContext::SWAPCHAIN_IMAGES; i++) {
        context.swapchainImageViews[i] = CreateImageView(context.swapchainImages[i], context.swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

  VkImageView VulkanGraphicsApplication::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
  {
       VkImageViewCreateInfo viewInfo{};
       viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
       viewInfo.image = image;
       viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
       viewInfo.format = format;
       viewInfo.subresourceRange.aspectMask = aspectFlags;
       viewInfo.subresourceRange.baseMipLevel = 0;
       viewInfo.subresourceRange.levelCount = mipLevels;
       viewInfo.subresourceRange.baseArrayLayer = 0;
       viewInfo.subresourceRange.layerCount = 1;
 
	   VkImageView imageView;
       if (vkCreateImageView(context.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
           throw std::runtime_error("failed to create texture image view!");
       }
       return imageView;
   }

  void VulkanGraphicsApplication::CreateTextureSampler()
  {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(context.physicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f; // Optional
		samplerInfo.maxLod = static_cast<float>(rendercontext.mipLevels);
		samplerInfo.mipLodBias = 0.0f; // Optional

        if (vkCreateSampler(context.device, &samplerInfo, nullptr, &rendercontext.textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

  void VulkanGraphicsApplication::CreateTextureNormalSampler()
  {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(context.physicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f; // Optional
		samplerInfo.maxLod = static_cast<float>(rendercontext.mipLevels);
		samplerInfo.mipLodBias = 0.0f; // Optional

        if (vkCreateSampler(context.device, &samplerInfo, nullptr, &rendercontext.textureNormalSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

  void VulkanGraphicsApplication::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
  {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspectFlags;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
} else {
    throw std::invalid_argument("transition d'organisation non supportée!");
}

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        EndSingleTimeCommands(commandBuffer);
    }

    void VulkanGraphicsApplication::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        EndSingleTimeCommands(commandBuffer);
    }

void VulkanGraphicsApplication::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) 
{
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(context.device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(context.device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = context.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(context.device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(context.device, image, imageMemory, 0);
}

void VulkanGraphicsApplication::LoadModel()
{
	tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

				vertex.normal = 
				{
					attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
				};

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                    mesh.vertices.push_back(vertex);
                }

                mesh.indices.push_back(uniqueVertices[vertex]);
            }
        }
}

VkCommandBuffer VulkanGraphicsApplication::BeginSingleTimeCommands() 
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = rendercontext.mainCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanGraphicsApplication::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(rendercontext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(rendercontext.graphicsQueue);

    vkFreeCommandBuffers(context.device, rendercontext.mainCommandPool, 1, &commandBuffer);
}

bool VulkanGraphicsApplication::Prepare()
{

	CreateRenderPath();

	CreateCommandPool();
	
	CreateCommandBuffer();
	

	VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (int i = 0; i < rendercontext.PENDING_FRAMES; i++)
    {
        DEBUG_CHECK_VK(vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &context.acquireSemaphores[i]));
        DEBUG_CHECK_VK(vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &context.presentSemaphores[i]));
    }

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (int i = 0; i < rendercontext.PENDING_FRAMES; i++)
    {
        DEBUG_CHECK_VK(vkCreateFence(context.device, &fenceInfo, nullptr, &rendercontext.mainFences[i]));
    }

	CreateDescriptorSetLayout();

	

	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();
	
	CreateTextureNormalImage();
	CreateTextureNormalImageView();
	CreateTextureNormalSampler();

	LoadModel();

	CreateDepthResources();
	CreateFrameBuffer();

	CreateGraphicsPipeline();
	
	CreateVertexBuffer();

	CreateIndexBuffer();

	CreateUniformBuffer();

	CreateDescriptorPool();

	CreateDescriptorSets();

	return true;
}

void VulkanGraphicsApplication::Terminate()
{
	for (int i = 0; i < rendercontext.PENDING_FRAMES; i++)
    {
        vkDestroyFence(context.device, rendercontext.mainFences[i], nullptr);
    }

    for (int i = 0; i < rendercontext.PENDING_FRAMES; i++)
    {
        vkDestroySemaphore(context.device, context.acquireSemaphores[i], nullptr);
        vkDestroySemaphore(context.device, context.presentSemaphores[i], nullptr);
    }

    for (int i = 0; i < rendercontext.PENDING_FRAMES; i++)
    {
        vkDestroyFramebuffer(context.device, rendercontext.framebuffers[i] , nullptr);
    }

    vkDestroyRenderPass(context.device, rendercontext.renderPass, nullptr);
    vkDestroyCommandPool(context.device, rendercontext.mainCommandPool, nullptr);
	vkDestroyBuffer(context.device, mesh.vertexBuffer, nullptr);
    vkFreeMemory(context.device, mesh.vertexBufferMemory, nullptr);

	vkDestroyBuffer(context.device, mesh.indexBuffer, nullptr);
    vkFreeMemory(context.device, mesh.indexBufferMemory, nullptr);

	for (int i = 0; i < rendercontext.uniformBuffers.size(); ++i)
	{
		vkDestroyBuffer(context.device, rendercontext.uniformBuffers[i], nullptr);
		vkFreeMemory(context.device, rendercontext.uniformBuffersMemory[i], nullptr);
	}

	vkDestroyPipeline(context.device, rendercontext.graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(context.device, rendercontext.pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(context.device, rendercontext.descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(context.device, rendercontext.descriptorPool, nullptr);

	vkDestroySampler(context.device, rendercontext.textureSampler, nullptr);
	vkDestroyImageView(context.device, rendercontext.textureImageView, nullptr);

    vkDestroyImage(context.device, rendercontext.textureImage, nullptr);
    vkFreeMemory(context.device, rendercontext.textureImageMemory, nullptr);
}

//
// Frame
//

bool VulkanGraphicsApplication::Begin()
{
	// render ---
	VkFence* commandFence = &rendercontext.mainFences[m_currentFrame];
	// on bloque tant que la fence soumis avec un command buffer n'a pas ete signale
	vkWaitForFences(context.device, 1, commandFence,
		false, UINT64_MAX);
	vkResetFences(context.device, 1, commandFence);

	vkResetCommandBuffer(rendercontext.mainCommandBuffers[m_currentFrame]
		, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	return true;
}

bool VulkanGraphicsApplication::End()
{
	VkSemaphore* acquireSem = &context.acquireSemaphores[m_currentFrame];
	VkSemaphore* presentSem = &context.presentSemaphores[m_currentFrame];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &rendercontext.mainCommandBuffers[m_currentFrame];
	VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.pWaitDstStageMask = &stageMask;
	submitInfo.pWaitSemaphores = acquireSem;	// on attend acquireSem
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = presentSem;	// on signal presentSem des qu'on est pret
	submitInfo.signalSemaphoreCount = 1;
	vkQueueSubmit(rendercontext.graphicsQueue, 1, &submitInfo
		, rendercontext.mainFences[m_currentFrame]);
	// la fence precedente se signale des que le command buffer est totalement traite par la queue

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = presentSem;// presentation semaphore ici synchro avec le dernier vkQueueSubmit
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &context.swapchain;
	presentInfo.pImageIndices = &m_imageIndex;
	DEBUG_CHECK_VK(vkQueuePresentKHR(rendercontext.presentQueue, &presentInfo));

	m_frame++;
	m_currentFrame = m_frame % rendercontext.PENDING_FRAMES;

	//vkDestroyCommandPool(context.device, rendercontext.mainCommandPool, nullptr);

	   // vkDestroyShaderModule(context.device, rendercontext.fragShaderModule, nullptr);
    //vkDestroyShaderModule(context.device, rendercontext.vertShaderModule, nullptr);

	return true;
}

bool VulkanGraphicsApplication::Update()
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	static double previousTime = glfwGetTime();
	static double currentTime = glfwGetTime();

	currentTime = glfwGetTime();
	double deltaTime = currentTime - previousTime;
	std::cout << "[" << m_frame << "] frame time = " << deltaTime*1000.0 << " ms [" << 1.0 / deltaTime << " fps]" << std::endl;
	previousTime = currentTime;

	float time = (float)currentTime;

	

	return true;
}

bool VulkanGraphicsApplication::Display()
{
	// COMMAND BUFFER SAFE ICI
	VkCommandBuffer commandBuffer = rendercontext.mainCommandBuffers[m_currentFrame];




	//- --

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = rendercontext.renderPass;
	renderPassInfo.framebuffer = rendercontext.framebuffers[m_currentFrame];

	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = context.swapchainExtent;

		std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{1.0f, 1.0f, 0.0f, 1.0f}};
	clearValues[1].depthStencil = {1.0f, 0};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	//VkClearValue clearColor = {{{1.0f, 1.0f, 0.0f, 1.0f}}};
	//renderPassInfo.clearValueCount = 1;
	//renderPassInfo.pClearValues = &clearColor;

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rendercontext.graphicsPipeline);

	VkBuffer vertexBuffers[] = {mesh.vertexBuffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rendercontext.pipelineLayout, 0, 1, &(rendercontext.descriptorSets[m_currentFrame]), 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);

	//vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);


	//vkCmdBindPipeline(rendercontext.mainCommandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, rendercontext.graphicsPipeline);
	//vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	
	vkCmdEndRenderPass(commandBuffer);


	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	// --

	VkSemaphore* acquireSem = &context.acquireSemaphores[m_currentFrame];
	VkSemaphore* presentSem = &context.presentSemaphores[m_currentFrame];

	uint64_t timeout = UINT64_MAX;
	DEBUG_CHECK_VK(vkAcquireNextImageKHR(context.device, context.swapchain
		, timeout, *acquireSem/*signal des que l'image est disponible */
		, VK_NULL_HANDLE, &m_imageIndex));

	 UpdateUniformBuffer(m_imageIndex, (float)glfwGetTime());

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	return true;
}

int main(void)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	VulkanGraphicsApplication app;

	/* Create a windowed mode window and its OpenGL context */
	app.window = glfwCreateWindow(1024, 768, APP_NAME, NULL, NULL);
	if (!app.window)
	{
		glfwTerminate();
		return -1;
	}

	app.Initialize(APP_NAME);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(app.window))
	{
		/* Render here */
		app.Run();

		/* Poll for and process events */
		glfwPollEvents();
	}

	app.Shutdown();

	glfwTerminate();
	return 0;
}
