#ifndef VK_INIT_HPP
#define VK_INIT_HPP
#include <vulkan/vulkan.h>

namespace vkh::init {
VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t qFIndex, VkCommandPoolCreateFlags flags = 0);

VkCommandBufferAllocateInfo CommandBufferAllocateInfo(
    VkCommandPool pool, uint32_t count = 1,
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

VkFramebufferCreateInfo FramebufferCreateInfo(VkRenderPass renderPass, VkExtent2D extent);

VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);

VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags);

VkSubmitInfo SubmitInfo(VkCommandBuffer* cmd);

VkPresentInfoKHR PresentInfo();

VkRenderPassBeginInfo RenderpassBeginInfo(VkRenderPass renderPass, VkExtent2D windowExtent,
                                          VkFramebuffer framebuffer);

VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
                                                              VkShaderModule shaderModule);

VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo();

VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo(
    VkPrimitiveTopology topology);

VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode polygonMode);

VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo();

VkPipelineColorBlendAttachmentState ColorBlendAttachmentState();

VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image,
                                          VkImageAspectFlags aspectFlags);

VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);

VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo(bool bDepthTest, bool bDepthWrite,
                                                                  VkCompareOp compareOp);

VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(VkDescriptorType type,
                                                        VkShaderStageFlags stageFlags,
                                                        uint32_t binding);
}  // namespace vkh::init
#endif  //VK_INIT_HPP
