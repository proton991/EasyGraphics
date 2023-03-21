#ifndef WSI_HPP
#define WSI_HPP
#include "ezg_util/timer.hpp"
#include "vk_base.hpp"

namespace ezg::vulkan {
class WSIPlatform {
public:
  virtual vk::SurfaceKHR CreateSurface(vk::UniqueInstance instance, vk::PhysicalDevice gpu) = 0;

  virtual std::vector<const char*> GetInstanceExts() = 0;
  virtual std::vector<const char*> GetDeviceExts() { return {"VK_KHR_swapchain"}; }

  virtual uint32_t GetWidth()  = 0;
  virtual uint32_t GetHeight() = 0;

  virtual float GetAspectRation() { return float(GetWidth()) / float(GetHeight()); }

  bool ShouldResize() const { return m_should_resize; }

  auto GetFrameTimer() const { return m_timer; }

protected:
  bool m_should_resize = false;

private:
  util::FrameTimer m_timer;
};
}  // namespace ezg::vulkan
#endif  //WSI_HPP
