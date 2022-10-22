#include "vk_swapchain.hpp"
#include "vk_functions.hpp"
#include "vk_tools.hpp"
namespace vkh {

SwapchainSupport QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
  SwapchainSupport details;
  VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice, surface, &details.capabilities);
  details.formats = GetVector<VkSurfaceFormatKHR>(
      VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceSurfaceFormatsKHR, physicalDevice,
      surface);
  details.presentModes = GetVector<VkPresentModeKHR>(
      VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceSurfacePresentModesKHR, physicalDevice,
      surface);
  return details;
}

VkSurfaceFormatKHR FindSurfaceFormat(VkPhysicalDevice physicalDevice,
                                     const std::vector<VkSurfaceFormatKHR>& availableFormats,
                                     const std::vector<VkSurfaceFormatKHR>& desiredFormats,
                                     VkFormatFeatureFlags featureFlags) {
  for (const auto& desiredFormat : desiredFormats) {
    for (const auto& availableFormat : availableFormats) {
      if (desiredFormat.format == availableFormat.format &&
          desiredFormat.colorSpace == availableFormat.colorSpace) {
        VkFormatProperties properties;
        VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceFormatProperties(
            physicalDevice, desiredFormat.format, &properties);
        if ((properties.optimalTilingFeatures & featureFlags) == featureFlags) {
          return desiredFormat;
        }
      }
    }
  }
  return availableFormats[0];
}

VkPresentModeKHR FindPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes,
                                 const std::vector<VkPresentModeKHR>& desiredPresentModes) {
  for (const auto& desired : desiredPresentModes) {
    for (const auto& available : availablePresentModes) {
      if (desired == available)
        return desired;
    }
  }
  return VK_PRESENT_MODE_MAILBOX_KHR;
}

VkExtent2D FindExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D actualExtent) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    // within the minImageExtent and maxImageExtent bounds.
    actualExtent.width = std::max(capabilities.minImageExtent.width,
                                  std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height =
        std::max(capabilities.minImageExtent.height,
                 std::min(capabilities.maxImageExtent.height, actualExtent.height));
  }
  return actualExtent;
}

std::vector<VkImageView> Swapchain::GetImageViews() {
  return GetImageViews(nullptr);
}

VkImageView Swapchain::CreateImageView(VkImage image, VkFormat format,
                                       VkImageAspectFlags aspectFlags) {
  VkImageView imageView;
  VkImageViewCreateInfo createInfo           = {};
  createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.image                           = image;
  createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format                          = format;
  createInfo.subresourceRange.aspectMask     = aspectFlags;
  createInfo.subresourceRange.baseMipLevel   = 0;
  createInfo.subresourceRange.levelCount     = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.layerCount     = 1;
  createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
  VkCheck(internalTable.fp_vkCreateImageView(device, &createInfo, allocationCallbacks, &imageView),
          "Create image view");
  return imageView;
}

std::vector<VkImageView> Swapchain::GetImageViews(const void* pNext) {
  auto images = GetImages();
  std::vector<VkImageView> imageViews(images.size());
  for (size_t i = 0; i < images.size(); i++) {
    VkImageViewCreateInfo createInfo           = {};
    createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext                           = pNext;
    createInfo.image                           = images[i];
    createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format                          = imageFormat;
    createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel   = 0;
    createInfo.subresourceRange.levelCount     = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = 1;

    VkCheck(internalTable.fp_vkCreateImageView(device, &createInfo, allocationCallbacks,
                                               &imageViews[i]),
            "Create image views");
  }
  return imageViews;
}

void Swapchain::DestroyImageViews(const std::vector<VkImageView>& imageViews) const {
  for (auto& imageView : imageViews) {
    internalTable.fp_vkDestroyImageView(device, imageView, allocationCallbacks);
  }
}

void Swapchain::DestroyImageView(const VkImageView& imageView) const {
  internalTable.fp_vkDestroyImageView(device, imageView, allocationCallbacks);
}

void DestroySwapchain(const Swapchain& swapchain) {
  if (swapchain.device != VK_NULL_HANDLE && swapchain.swapchain != VK_NULL_HANDLE) {
    swapchain.internalTable.fp_vkDestroySwapchainKHR(swapchain.device, swapchain.swapchain,
                                                     swapchain.allocationCallbacks);
  }
}

SwapchainBuilder::SwapchainBuilder(const Device& device) {
  info.device                 = device;
  info.physicalDevice         = device.physicalDevice.physicalDevice;
  info.surface                = device.surface;
  uint32_t presentQueueIndex  = device.GetQueueIndex(QueueType::present);
  uint32_t graphicsQueueIndex = device.GetQueueIndex(QueueType::graphics);
  info.graphicsQueueIndex     = graphicsQueueIndex;
  info.presentQueueIndex      = presentQueueIndex;
  info.allocationCallbacks    = device.allocationCallbacks;
}
SwapchainBuilder::SwapchainBuilder(const Device& device, const VkSurfaceKHR surface) {
  info.device                 = device;
  info.physicalDevice         = device.physicalDevice.physicalDevice;
  info.surface                = surface;
  Device tmpDevice            = device;
  tmpDevice.surface           = surface;
  uint32_t presentQueueIndex  = tmpDevice.GetQueueIndex(QueueType::present);
  uint32_t graphicsQueueIndex = tmpDevice.GetQueueIndex(QueueType::graphics);
  info.graphicsQueueIndex     = graphicsQueueIndex;
  info.presentQueueIndex      = presentQueueIndex;
  info.allocationCallbacks    = device.allocationCallbacks;
}

SwapchainBuilder::SwapchainBuilder(const VkPhysicalDevice physicalDevice, const VkDevice device,
                                   const VkSurfaceKHR surface, uint32_t graphicsQueueIndex,
                                   uint32_t presentQueueIndex) {
  info.physicalDevice     = physicalDevice;
  info.device             = device;
  info.surface            = surface;
  info.graphicsQueueIndex = graphicsQueueIndex;
  info.presentQueueIndex  = graphicsQueueIndex;

  if (graphicsQueueIndex == QUEUE_INDEX_MAX_VALUE || presentQueueIndex == QUEUE_INDEX_MAX_VALUE) {
    throw std::runtime_error("Graphics or present queue index not provided when build swapchain");
  }
}

Swapchain SwapchainBuilder::Build() {
  if (info.surface == VK_NULL_HANDLE) {
    throw std::runtime_error("Try to create swapchain when surface is null!");
  }
  auto formats = info.desiredFormats;
  if (formats.empty())
    AddDefaultFormats();
  auto presentModes = info.desiredPresentModes;
  if (presentModes.empty())
    AddDefaultPresentModes();
  SwapchainSupport swapchainSupport = QuerySwapchainSupport(info.physicalDevice, info.surface);
  uint32_t imageCount = std::max(info.minImageCount, swapchainSupport.capabilities.minImageCount);
  if (swapchainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapchainSupport.capabilities.maxImageCount) {
    imageCount = swapchainSupport.capabilities.maxImageCount;
  }
  VkSurfaceFormatKHR surfaceFormat = FindSurfaceFormat(
      info.physicalDevice, swapchainSupport.formats, formats, info.formatFeatureFlags);

  VkExtent2D extent2D = FindExtent(swapchainSupport.capabilities, info.extent2D);
  uint32_t imageArrayLayers =
      std::max(info.arrayLayerCount, swapchainSupport.capabilities.maxImageArrayLayers);
  if (info.arrayLayerCount == 0)
    imageArrayLayers = 1;

  uint32_t queueFamilyIndices[] = {info.graphicsQueueIndex, info.presentQueueIndex};
  VkPresentModeKHR presentMode  = FindPresentMode(swapchainSupport.presentModes, presentModes);

  VkSurfaceTransformFlagBitsKHR preTransform = info.preTransform;
  if (info.preTransform == static_cast<VkSurfaceTransformFlagBitsKHR>(0))
    preTransform = swapchainSupport.capabilities.currentTransform;

  VkSwapchainCreateInfoKHR swapchainCreateInfo{};
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  SetPNextChain(swapchainCreateInfo, info.pNextChain);
#if !defined(NDEBUG)
  for (auto& node : info.pNextChain) {
    assert(node->sType != VK_STRUCTURE_TYPE_APPLICATION_INFO);
  }
#endif
  swapchainCreateInfo.flags            = info.createFlags;
  swapchainCreateInfo.surface          = info.surface;
  swapchainCreateInfo.minImageCount    = imageCount;
  swapchainCreateInfo.imageFormat      = surfaceFormat.format;
  swapchainCreateInfo.imageColorSpace  = surfaceFormat.colorSpace;
  swapchainCreateInfo.imageExtent      = extent2D;
  swapchainCreateInfo.imageArrayLayers = imageArrayLayers;
  swapchainCreateInfo.imageUsage       = info.imageUsageFlags;

  if (info.graphicsQueueIndex != info.presentQueueIndex) {
    swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
  } else {
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  swapchainCreateInfo.preTransform   = preTransform;
  swapchainCreateInfo.compositeAlpha = info.compositeAlpha;
  swapchainCreateInfo.presentMode    = presentMode;
  swapchainCreateInfo.clipped        = info.clipped;
  swapchainCreateInfo.oldSwapchain   = info.oldSwapchain;
  PFN_vkCreateSwapchainKHR swapchainCreateProc;
  VulkanFunction::GetInstance().GetDeviceProcAddr(info.device, swapchainCreateProc,
                                                  "vkCreateSwapchainKHR");
  Swapchain swapchain{};
  VkCheck(swapchainCreateProc(info.device, &swapchainCreateInfo, info.allocationCallbacks,
                              &swapchain.swapchain),
          "Create swapchain");
  VulkanFunction::GetInstance().GetDeviceProcAddr(
      info.device, swapchain.internalTable.fp_vkGetSwapchainImagesKHR, "vkGetSwapchainImagesKHR");
  VulkanFunction::GetInstance().GetDeviceProcAddr(
      info.device, swapchain.internalTable.fp_vkCreateImageView, "vkCreateImageView");
  VulkanFunction::GetInstance().GetDeviceProcAddr(
      info.device, swapchain.internalTable.fp_vkDestroyImageView, "vkDestroyImageView");
  VulkanFunction::GetInstance().GetDeviceProcAddr(
      info.device, swapchain.internalTable.fp_vkDestroySwapchainKHR, "vkDestroySwapchainKHR");
  swapchain.device              = info.device;
  swapchain.imageFormat         = surfaceFormat.format;
  swapchain.colorSpace          = surfaceFormat.colorSpace;
  swapchain.extent2D            = extent2D;
  swapchain.minImageCount       = imageCount;
  swapchain.presentMode         = presentMode;
  swapchain.allocationCallbacks = info.allocationCallbacks;

  VkCheck(GetVector<VkImage>(swapchain.images, swapchain.internalTable.fp_vkGetSwapchainImagesKHR,
                             swapchain.device, swapchain),
          "Get swapchain images");
  swapchain.imageCount = static_cast<uint32_t>(swapchain.images.size());
  return swapchain;
}
SwapchainBuilderRef SwapchainBuilder::SetOldSwapchain(VkSwapchainKHR oldSwapchain) {
  info.oldSwapchain = oldSwapchain;
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::SetOldSwapchain(const Swapchain& oldSwapchain) {
  info.oldSwapchain = oldSwapchain.swapchain;
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::SetExtent(uint32_t width, uint32_t height) {
  info.extent2D = {width, height};
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::AddDesiredFormat(VkSurfaceFormatKHR format) {
  // format with lower index has higher priority
  info.desiredFormats.insert(info.desiredFormats.begin(), format);
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::UseDefaultFormat() {
  info.desiredFormats.clear();
  AddDefaultFormats();
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::SetDesiredPresentMode(VkPresentModeKHR presentMode) {
  info.desiredPresentModes.insert(info.desiredPresentModes.begin(), presentMode);
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::UseDefaultPresentMode() {
  info.desiredPresentModes.clear();
  AddDefaultPresentModes();
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::SetImageUseFlags(VkImageUsageFlags flags) {
  info.imageUsageFlags = flags;
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::AddImageUseFlags(VkImageUsageFlags flags) {
  info.imageUsageFlags |= flags;
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::UseDefaultImageUseFlags() {
  info.imageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::SetFormatFeatureFlags(VkFormatFeatureFlags flags) {
  info.formatFeatureFlags = flags;
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::AddFormatFeatureFlags(VkFormatFeatureFlags flags) {
  info.formatFeatureFlags |= flags;
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::UseDefaultFormatFeatureFlags() {
  info.formatFeatureFlags = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::SetImageArrayLayerCount(uint32_t count) {
  info.arrayLayerCount = count;
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::SetMinImageCount(uint32_t count) {
  info.minImageCount = count;
  return *this;
}
SwapchainBuilderRef SwapchainBuilder::SetClipped(bool clipped) {
  info.clipped = clipped;
  return *this;
}
SwapchainBuilder& SwapchainBuilder::SetCreateFlags(VkSwapchainCreateFlagBitsKHR flags) {
  info.createFlags = flags;
  return *this;
}
SwapchainBuilder& SwapchainBuilder::SetPreTransformFlags(
    VkSurfaceTransformFlagBitsKHR pre_transform_flags) {
  info.preTransform = pre_transform_flags;
  return *this;
}
SwapchainBuilder& SwapchainBuilder::SetCompositeAlphaFlags(VkCompositeAlphaFlagBitsKHR flags) {
  info.compositeAlpha = flags;
  return *this;
}
SwapchainBuilder& SwapchainBuilder::SetAllocationCallbacks(VkAllocationCallbacks* callbacks) {
  info.allocationCallbacks = callbacks;
  return *this;
}
void SwapchainBuilder::AddDefaultFormats() {
  info.desiredFormats.push_back({VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
  info.desiredFormats.push_back({VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
}
void SwapchainBuilder::AddDefaultPresentModes() {
  info.desiredPresentModes.push_back(VK_PRESENT_MODE_MAILBOX_KHR);
  info.desiredPresentModes.push_back(VK_PRESENT_MODE_FIFO_KHR);
}
}  // namespace vkh