#ifndef RENDER_PASS_HPP
#define RENDER_PASS_HPP
#include <volk.h>
#include <array>
#include <optional>
#include <vector>

namespace ezg::vk {
class Device;
class RenderStage;

enum class AttachmentType { Color, DepthStencil, Swapchain };
class Attachment {
public:
  Attachment(AttachmentType type, uint32_t binding, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
             VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
             VkClearColorValue clearColor  = {0.0f, 0.0f, 0.0f, 1.0f});

  [[nodiscard]] AttachmentType GetType() const { return m_type; }
  [[nodiscard]] VkFormat GetFormat() const { return m_format; }
  [[nodiscard]] VkSampleCountFlagBits GetSamples() const { return m_samples; }
  [[nodiscard]] uint32_t GetBinding() const { return m_binding; }
  [[nodiscard]] VkClearColorValue GetClearColor() const { return m_clearColor; }

private:
  AttachmentType m_type;
  uint32_t m_binding;
  VkFormat m_format;
  VkSampleCountFlagBits m_samples;
  VkClearColorValue m_clearColor;
};

struct SubpassInfo {
  uint32_t binding{};
  std::vector<uint32_t> colorAttachments{};
  uint32_t depthStencilAttachment{};
};

class RenderPass {
public:
  RenderPass(const Device& device, std::vector<SubpassInfo> subpassInfos,
             std::vector<Attachment> attachments);

  ~RenderPass();

  [[nodiscard]] VkRenderPass Handle() const { return m_renderPass; }

  [[nodiscard]] const std::vector<Attachment>& GetAttachments() const { return m_attachments; }
  [[nodiscard]] uint32_t GetAttachmentCount() const { return m_attachments.size(); }

private:
  const Device& m_device;
  VkRenderPass m_renderPass{VK_NULL_HANDLE};

  std::vector<Attachment> m_attachments;
  std::optional<Attachment> m_depthAttachment;
  std::optional<Attachment> m_swapchainAttachment;

  std::vector<SubpassInfo> m_subpassInfos;
};
}  // namespace ezg::vk
#endif  //RENDER_PASS_HPP
