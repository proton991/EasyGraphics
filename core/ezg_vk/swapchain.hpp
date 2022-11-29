#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include <volk.h>
#include <optional>
#include <vector>
#include "common.hpp"
#include "wsi.hpp"

namespace ezg::vk {
class Device;
struct SwapchainSupportDetail {
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};
class Swapchain {
public:
  NO_COPY(Swapchain)
  NO_MOVE(Swapchain)
  Swapchain(const wsi::Platform* platform, VkInstance instance, const Device* device,
            uint32_t width, uint32_t height, bool enableVsync = true);
  ~Swapchain();

  uint32_t AcquireNextImage(VkSemaphore semaphore);

  VkSwapchainKHR Handle() { return m_swapchain; }

  VkSurfaceKHR Surface() { return m_surface; }

  void Recreate(uint32_t windowWidth, uint32_t windowHeight);

  void QueuePresent(uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

private:
  void Setup(VkSwapchainKHR oldSwapchain, uint32_t width, uint32_t height);
  void QuerySwapchainSupport();
  void DecideSwapchainExtent(uint32_t windowWidth, uint32_t windowHeight);
  void DecideImageCount();
  void DecideSurfaceColorFormat();

  std::optional<VkPresentModeKHR> ChoosePresentMode();
  std::optional<VkCompositeAlphaFlagBitsKHR> FindCompositeAlphaFormat();

private:
  const Device* m_device{nullptr};
  const wsi::Platform* m_sdl2Platform{nullptr};
  VkSurfaceKHR m_surface{VK_NULL_HANDLE};
  VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
  SwapchainSupportDetail m_details{};
  VkSurfaceFormatKHR m_surfaceFormat{};
  uint32_t m_imageCount{0};
  VkExtent2D m_extent2D{};
  std::vector<VkImage> m_images;
  std::vector<VkImageView> m_imageViews;
  bool m_vsyncEnabled{true};
};
}  // namespace ezg::vk

#endif  //SWAPCHAIN_HPP
