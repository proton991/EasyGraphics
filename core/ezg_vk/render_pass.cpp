#include "render_pass.hpp"
#include "device.hpp"

namespace ezg::vk {

Attachment::Attachment(AttachmentType type, uint32_t binding, VkFormat format,
                       VkSampleCountFlagBits samples, VkClearColorValue clearColor)
    : m_type(type),
      m_binding(binding),
      m_format(format),
      m_samples(samples),
      m_clearColor(clearColor) {}

RenderPass::RenderPass(const Device& device, std::vector<SubpassInfo> subpassInfos,
                       std::vector<Attachment> attachments)
    : m_device(device),
      m_attachments(std::move(attachments)),
      m_subpassInfos(std::move(subpassInfos)) {
  // sort attachments based on bindings
  std::sort(m_attachments.begin(), m_attachments.end(),
            [](const Attachment& a, const Attachment& b) -> bool {
              return a.GetBinding() < b.GetBinding();
            });
  std::vector<VkAttachmentDescription> attachmentDescriptions;
  attachmentDescriptions.reserve(m_attachments.size());
  for (const auto& attachment : m_attachments) {
    VkAttachmentDescription description = {.format         = attachment.GetFormat(),
                                           .samples        = attachment.GetSamples(),
                                           .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                                           .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                           .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED};

    switch (attachment.GetType()) {
      case AttachmentType::Color:
        description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        break;
      case AttachmentType::DepthStencil:
        description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        break;
      case AttachmentType::Swapchain:
        description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        break;
    }
    attachmentDescriptions.emplace_back(description);
  }

  // config subpass dependencies
  std::vector<VkSubpassDependency> dependencies;
  std::vector<VkSubpassDescription> subpassDescriptions;
  for (const auto& subpassInfo : m_subpassInfos) {
    // Subpass dependencies.
    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask        = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subpassDependency.srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependency.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
    subpassDependency.dependencyFlags     = VK_DEPENDENCY_BY_REGION_BIT;

    if (subpassInfo.binding == m_subpassInfos.size()) {
      subpassDependency.dstSubpass   = VK_SUBPASS_EXTERNAL;
      subpassDependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      subpassDependency.srcAccessMask =
          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      subpassDependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    } else {
      subpassDependency.dstSubpass = subpassInfo.binding;
    }

    if (subpassInfo.binding == 0) {
      subpassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
      subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      subpassDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      subpassDependency.dstAccessMask =
          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    } else {
      subpassDependency.srcSubpass = subpassInfo.binding - 1;
    }
    dependencies.emplace_back(subpassDependency);

    // Subpass Attachments Reference
    std::vector<VkAttachmentReference> subpassColorAttachments;
    for (const auto& colorAttachment : subpassInfo.colorAttachments) {
      VkAttachmentReference attachmentReference = {
          .attachment = colorAttachment, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

      subpassColorAttachments.emplace_back(attachmentReference);
    }
    VkSubpassDescription subpassDescription = {
        .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<uint32_t>(subpassColorAttachments.size()),
        .pColorAttachments    = subpassColorAttachments.data(),
    };
    VkAttachmentReference depthStencilAttachment = {
        .attachment = subpassInfo.depthStencilAttachment,
        .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachment;
    subpassDescriptions.emplace_back(subpassDescription);
  }
  // Creates the render pass.
  VkRenderPassCreateInfo renderPassCreateInfo = {
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size()),
      .pAttachments    = attachmentDescriptions.data(),
      .subpassCount    = static_cast<uint32_t>(subpassDescriptions.size()),
      .pSubpasses      = subpassDescriptions.data(),
      .dependencyCount = static_cast<uint32_t>(dependencies.size()),
      .pDependencies   = dependencies.data()};
  Check(vkCreateRenderPass(m_device.Handle(), &renderPassCreateInfo, nullptr, &m_renderPass),
        "create renderpass");
}
RenderPass::~RenderPass() {
  vkDestroyRenderPass(m_device.Handle(), m_renderPass, nullptr);
}

}  // namespace ezg::vk