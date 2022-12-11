#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP
#include "common.hpp"
#include "image.hpp"

namespace ezg::vk {
class Device;
class Swapchain;
class RenderStage;
class RenderPass;

class Framebuffers {
public:
  NO_COPY(Framebuffers)

  Framebuffers(const Device& device, const Swapchain& swapchain, const RenderPass& renderPass,
               const Image& depthStencilImage);

  ~Framebuffers();

  [[nodiscard]] const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_framebuffers; }

private:
  const Device& m_device;
  std::vector<std::unique_ptr<Image>> m_imageAttachments;
  std::vector<VkFramebuffer> m_framebuffers;
};
}  // namespace ezg::vk
#endif  //FRAMEBUFFER_HPP
