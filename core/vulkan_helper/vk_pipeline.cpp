#include "vk_pipeline.hpp"
#include "vk_tools.hpp"
namespace vkh {
VkPipeline PipelineBuilder::Build(VkDevice device, VkRenderPass pass) {
  //make viewport state from our stored viewport and scissor.
  //at the moment we wont support multiple viewports or scissors
  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.pNext = nullptr;

  viewportState.viewportCount = 1;
  viewportState.pViewports    = &m_viewport;
  viewportState.scissorCount  = 1;
  viewportState.pScissors     = &m_scissor;

  //setup dummy color blending. We arent using transparent objects yet
  //the blending is just "no blend", but we do write to the color attachment
  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.pNext = nullptr;

  colorBlending.logicOpEnable   = VK_FALSE;
  colorBlending.logicOp         = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments    = &m_colorBlendAttachment;

  //build the actual pipeline
  //we now use all of the info structs we have been writing into into this one to create the pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.pNext = nullptr;

  pipelineInfo.stageCount          = m_shaderStages.size();
  pipelineInfo.pStages             = m_shaderStages.data();
  pipelineInfo.pVertexInputState   = &m_vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &m_inputAssembly;
  pipelineInfo.pViewportState      = &viewportState;
  pipelineInfo.pRasterizationState = &m_rasterizer;
  pipelineInfo.pMultisampleState   = &m_multisampling;
  pipelineInfo.pColorBlendState    = &colorBlending;
  pipelineInfo.layout              = m_pipelineLayout;
  pipelineInfo.renderPass          = pass;
  pipelineInfo.subpass             = 0;
  pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
  pipelineInfo.pDepthStencilState  = &m_depthStencil;

  //its easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
  VkPipeline newPipeline;
  if (fp_vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                   &newPipeline) != VK_SUCCESS) {
    vkh::Log("Failed to create pipeline!");
    return VK_NULL_HANDLE;  // failed to create graphics pipeline
  } else {
    return newPipeline;
  }
}
}  // namespace vkh