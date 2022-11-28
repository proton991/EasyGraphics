#include "swapchain.hpp"
#include "device.hpp"

namespace ezg::vk {
Swapchain::Swapchain(const wsi::Platform* platform, VkInstance instance, const Device* device,
                     uint32_t width, uint32_t height, bool enableVsync)
    : m_device(device), m_sdl2Platform(platform), m_vsyncEnabled(enableVsync) {
  assert(m_device->Handle());
  m_surface = m_sdl2Platform->CreateSurface(instance);
  DebugUtil::Get().SetObjectName(m_surface, "surface");
  QuerySwapchainSupport();
  Setup(VK_NULL_HANDLE, width, height);
}

Swapchain::~Swapchain() {
  if (m_swapchain != nullptr) {
    vkDestroySwapchainKHR(m_device->Handle(), m_swapchain, nullptr);
  }
  for (auto& imageView : m_imageViews) {
    vkDestroyImageView(m_device->Handle(), imageView, nullptr);
  }
  if (m_surface != nullptr) {
    vkDestroySurfaceKHR(m_device->Instance(), m_surface, nullptr);
  }
}

uint32_t Swapchain::AcquireNextImage(VkSemaphore semaphore) {
  uint32_t imageIndex = 0;
  vkAcquireNextImageKHR(m_device->Handle(), m_swapchain, std::numeric_limits<std::uint64_t>::max(),
                        semaphore, VK_NULL_HANDLE, &imageIndex);
  return imageIndex;
}

void Swapchain::QuerySwapchainSupport() {
  // query capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->GPU(), m_surface, &m_details.capabilities);

  // query present modes
  uint32_t modeCount = 0;
  Check(vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->GPU(), m_surface, &modeCount, nullptr),
        "get present mode count");
  m_details.presentModes.resize(modeCount);
  Check(vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->GPU(), m_surface, &modeCount,
                                                  m_details.presentModes.data()),
        "get present mode data");

  uint32_t surfaceFormatCount = 0;
  Check(vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->GPU(), m_surface, &surfaceFormatCount,
                                             nullptr),
        "get surface format count");
  m_details.formats.resize(surfaceFormatCount);
  Check(vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->GPU(), m_surface, &surfaceFormatCount,
                                             m_details.formats.data()),
        "get surface format data");
}

void Swapchain::DecideSwapchainExtent(uint32_t windowWidth, uint32_t windowHeight) {
  VkExtent2D actualExtent{.width = windowWidth, .height = windowHeight};
  VkSurfaceCapabilitiesKHR capabilities{};
  Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->GPU(), m_surface, &capabilities),
        "get surface capabilities");
  // within the minImageExtent and maxImageExtent bounds.
  m_extent2D.width  = std::max(capabilities.minImageExtent.width,
                               std::min(capabilities.maxImageExtent.width, actualExtent.width));
  m_extent2D.height = std::max(capabilities.minImageExtent.height,
                               std::min(capabilities.maxImageExtent.height, actualExtent.height));
}

std::optional<VkPresentModeKHR> Swapchain::ChoosePresentMode() {
  for (auto present_mode : m_details.presentModes) {
    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
    // VK_PRESENT_MODE_MAILBOX_KHR specifies that the presentation engine waits for the next vertical blanking
    // period to update the current image. Tearing cannot be observed. An internal single-entry queue is used to
    // hold pending presentation requests. If the queue is full when a new presentation request is received, the new
    // request replaces the existing entry, and any images associated with the prior entry become available for
    // re-use by the application. One request is removed from the queue and processed during each vertical blanking
    // period in which the queue is non-empty.
    if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR && m_vsyncEnabled) {
      spdlog::info("VK_PRESENT_MODE_MAILBOX_KHR will be used");
      return VK_PRESENT_MODE_MAILBOX_KHR;
    }
  }

  for (auto present_mode : m_details.presentModes) {
    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
    // VK_PRESENT_MODE_IMMEDIATE_KHR specifies that the presentation engine does not wait for a vertical blanking
    // period to update the current image, meaning this mode may result in visible tearing. No internal queuing of
    // presentation requests is needed, as the requests are applied immediately.
    if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      spdlog::info("VK_PRESENT_MODE_IMMEDIATE_KHR will be used");
      return VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
  }

  spdlog::warn("VK_PRESENT_MODE_IMMEDIATE_KHR is not supported by the regarded device");
  spdlog::warn("Let's see if VK_PRESENT_MODE_FIFO_KHR is supported");

  for (auto present_mode : m_details.presentModes) {
    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
    // VK_PRESENT_MODE_FIFO_KHR specifies that the presentation engine waits for the next vertical blanking period
    // to update the current image. Tearing cannot be observed. An internal queue is used to hold pending
    // presentation requests. New requests are appended to the end of the queue, and one request is removed from the
    // beginning of the queue and processed during each vertical blanking period in which the queue is non-empty.
    // This is the only value of presentMode that is required to be supported.
    if (present_mode == VK_PRESENT_MODE_FIFO_KHR) {
      spdlog::info("VK_PRESENT_MODE_FIFO_KHR will be used");
      return VK_PRESENT_MODE_FIFO_KHR;
    }
  }

  spdlog::error("VK_PRESENT_MODE_FIFO_KHR is not supported by the regarded device");
  spdlog::error("According to the Vulkan specification, this shouldn't even be possible!");

  // Lets try with any present mode available!
  if (!m_details.presentModes.empty()) {
    // Let's just pick the first one.
    return m_details.presentModes[0];
  }

  // Yes, this might be the case for integrated systems!
  spdlog::critical("The selected graphics card does not support any presentation at all!");

  return std::nullopt;
}

void Swapchain::DecideImageCount() {
  m_imageCount = m_details.capabilities.minImageCount + 1;
  if ((m_details.capabilities.maxImageCount > 0) &&
      (m_details.capabilities.maxImageCount < m_imageCount)) {
    m_imageCount = m_details.capabilities.maxImageCount;
  }
}

void Swapchain::DecideSurfaceColorFormat() {
  if (m_details.formats.empty()) {
    throw std::runtime_error(
        "Error: No surface formats could be found by fpGetPhysicalDeviceSurfaceFormatsKHR!");
  }

  if (m_details.formats.size() == 1 && m_details.formats[0].format == VK_FORMAT_UNDEFINED) {
    m_surfaceFormat.format     = VK_FORMAT_B8G8R8A8_UNORM;
    m_surfaceFormat.colorSpace = m_details.formats[0].colorSpace;
  } else {
    std::vector<VkFormat> defaultFormats = {
        VK_FORMAT_B8G8R8A8_UNORM
        // TODO: Add more accepted formats here..
    };
    // Loop through the list of available surface formats and compare with the list of acceptable formats.
    for (auto& surfaceFormat : m_details.formats) {
      for (auto& acceptedFormat : defaultFormats) {
        if (surfaceFormat.format == acceptedFormat) {
          m_surfaceFormat.format     = surfaceFormat.format;
          m_surfaceFormat.colorSpace = surfaceFormat.colorSpace;
          return;
        }
      }
    }
    if (!m_details.formats.empty()) {
      m_surfaceFormat = m_details.formats[0];
    }
  }
}

std::optional<VkCompositeAlphaFlagBitsKHR> Swapchain::FindCompositeAlphaFormat() {
  const std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
  };
  for (const auto& flag : compositeAlphaFlags) {
    if ((m_details.capabilities.supportedCompositeAlpha & flag) != 0) {
      return flag;
    }
  }
  return std::nullopt;
}

void Swapchain::Recreate(uint32_t windowWidth, uint32_t windowHeight) {
  // Store the old swapchain. This allows us to pass it to VkSwapchainCreateInfoKHR::oldSwapchain to speed up swapchain recreation.
  VkSwapchainKHR oldSwapchain = m_swapchain;
  // When swapchain needs to be recreated, all the old swapchain images need to be destroyed.

  // Unlike swapchain images, the image views were created by us directly.
  // It is our job to destroy them again.
  for (auto* image_view : m_imageViews) {
    vkDestroyImageView(m_device->Handle(), image_view, nullptr);
  }

  m_imageViews.clear();

  m_images.clear();

  Setup(oldSwapchain, windowWidth, windowHeight);
}

void Swapchain::Setup(VkSwapchainKHR oldSwapchain, uint32_t width, uint32_t height) {
  DecideSwapchainExtent(width, height);
  std::optional<VkPresentModeKHR> presentMode = ChoosePresentMode();
  if (!presentMode) {
    throw std::runtime_error("Error: Could not find a suitable present mode!");
  }
  DecideImageCount();
  DecideSurfaceColorFormat();
  const auto& compositeAlpha = FindCompositeAlphaFormat();
  if (!compositeAlpha) {
    throw std::runtime_error(
        "Error: Could not find composite alpha format while recreating swapchain!");
  }

  VkSwapchainCreateInfoKHR swapchainInfo = {
      .sType           = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface         = m_surface,
      .minImageCount   = m_imageCount,
      .imageFormat     = m_surfaceFormat.format,
      .imageColorSpace = m_surfaceFormat.colorSpace,
      .imageExtent{
          .width  = m_extent2D.width,
          .height = m_extent2D.height,
      },
      .imageArrayLayers      = 1,
      .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices   = nullptr,
      .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha        = compositeAlpha.value(),
      .presentMode           = presentMode.value(),
      .clipped               = VK_TRUE,
      .oldSwapchain          = oldSwapchain,
  };

  // Set additional usage flag for blitting from the swapchain images if supported.
  VkFormatProperties formatProps;

  vkGetPhysicalDeviceFormatProperties(m_device->GPU(), m_surfaceFormat.format, &formatProps);

  if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) != 0 ||
      (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) != 0) {
    swapchainInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }

  Check(vkCreateSwapchainKHR(m_device->Handle(), &swapchainInfo, nullptr, &m_swapchain),
        "create swapchain");

  Check(vkGetSwapchainImagesKHR(m_device->Handle(), m_swapchain, &m_imageCount, nullptr),
        "get swapchain image count");
  m_images.resize(m_imageCount);
  Check(vkGetSwapchainImagesKHR(m_device->Handle(), m_swapchain, &m_imageCount, m_images.data()),
        "get swapchain image data");
  DebugUtil::Get().SetObjectName(m_swapchain, "swapchain");

  spd::info("Creating {} swapchain image views", m_imageCount);

  m_imageViews.resize(m_imageCount);
  VkImageViewCreateInfo imageViewInfo = {
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format   = m_surfaceFormat.format,
      .components{
          .r = VK_COMPONENT_SWIZZLE_IDENTITY,
          .g = VK_COMPONENT_SWIZZLE_IDENTITY,
          .b = VK_COMPONENT_SWIZZLE_IDENTITY,
          .a = VK_COMPONENT_SWIZZLE_IDENTITY,
      },
      .subresourceRange{
          .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel   = 0,
          .levelCount     = 1,
          .baseArrayLayer = 0,
          .layerCount     = 1,
      },
  };
  for (auto i = 0U; i < m_imageCount; i++) {
    imageViewInfo.image = m_images[i];
    vkCreateImageView(m_device->Handle(), &imageViewInfo, nullptr, &m_imageViews[i]);
  }
}

}  // namespace ezg::vk