#include "framebuffer.hpp"
#include "device.hpp"
#include "render_pass.hpp"
#include "swapchain.hpp"

namespace ezg::vk {

Framebuffers::Framebuffers(const Device& device, const Swapchain& swapchain,
                           const RenderPass& renderPass, const Image& depthStencilImage)
    : m_device(device) {
  m_imageAttachments.reserve(renderPass.GetAttachmentCount());
  for (const auto& attachment : renderPass.GetAttachments()) {
    // create image attachment
    switch (attachment.GetType()) {
      case AttachmentType::Color: {
        ImageCreateInfo imageInfo = {.format        = attachment.GetFormat(),
                                     .usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                     .aspect        = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .samples       = attachment.GetSamples(),
                                     .vmaAllocFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                                     .name          = "image",
                                     .imageExtent   = swapchain.GetExtent2D()};
        m_imageAttachments.emplace_back(std::make_unique<Image>(device, imageInfo));
        break;
      }
      case AttachmentType::DepthStencil:
      case AttachmentType::Swapchain:
        m_imageAttachments.emplace_back(nullptr);
        break;
    }
  }
  m_framebuffers.resize(swapchain.GetImageCount());
  for (auto i = 0U; i < swapchain.GetImageCount(); i++) {
    std::vector<VkImageView> attachments;
    for (const auto& attachment : renderPass.GetAttachments()) {
      switch (attachment.GetType()) {
        case AttachmentType::Color:
          attachments.emplace_back(m_imageAttachments[attachment.GetBinding()]->GetView());
          break;
        case AttachmentType::DepthStencil:
          attachments.emplace_back(depthStencilImage.GetView());
          break;
        case AttachmentType::Swapchain:
          attachments.emplace_back(swapchain.GetImageViewAt(i));
          break;
      }
    }
    VkFramebufferCreateInfo framebufferCreateInfo = {
        .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext           = nullptr,
        .renderPass      = renderPass.Handle(),
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments    = attachments.data(),
        .width           = swapchain.GetExtent2D().width,
        .height          = swapchain.GetExtent2D().height,
        .layers          = 1,
    };
    Check(vkCreateFramebuffer(device.Handle(), &framebufferCreateInfo, nullptr, &m_framebuffers[i]),
          "create framebuffer");
  }
}
Framebuffers::~Framebuffers() {
  for (const auto& framebuffer : m_framebuffers) {
    vkDestroyFramebuffer(m_device.Handle(), framebuffer, nullptr);
  }
}
}  // namespace ezg::vk