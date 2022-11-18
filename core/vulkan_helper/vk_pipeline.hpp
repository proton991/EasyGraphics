#ifndef VK_PIPELINE_HPP
#define VK_PIPELINE_HPP
#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vulkan/vulkan.h>
#include <vector>
namespace vkh {

class PipelineBuilder {
public:
  PipelineBuilder(PFN_vkCreateGraphicsPipelines fp_vkCreateGraphicsPipelines_) {
    fp_vkCreateGraphicsPipelines = fp_vkCreateGraphicsPipelines_;
  }
  std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
  VkPipelineVertexInputStateCreateInfo m_vertexInputInfo;
  VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;
  VkViewport m_viewport;
  VkRect2D m_scissor;
  VkPipelineRasterizationStateCreateInfo m_rasterizer;
  VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
  VkPipelineMultisampleStateCreateInfo m_multisampling;
  VkPipelineLayout m_pipelineLayout;
  VkPipelineDepthStencilStateCreateInfo m_depthStencil;

  VkPipeline Build(VkDevice device, VkRenderPass pass);

private:
  PFN_vkCreateGraphicsPipelines fp_vkCreateGraphicsPipelines;
};

}  // namespace ezg
#endif  //PIPELINE_HPP

#endif  //VK_PIPELINE_HPP
