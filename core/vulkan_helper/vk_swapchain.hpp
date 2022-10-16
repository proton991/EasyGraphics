#ifndef VK_SWAPCHAIN_HPP
#define VK_SWAPCHAIN_HPP

#include "vk_device.hpp"

namespace vkh {
struct SwapchainSupport {
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

SwapchainSupport QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

VkSurfaceFormatKHR FindSurfaceFormat(VkPhysicalDevice physicalDevice,
                                     const std::vector<VkSurfaceFormatKHR>& availableFormats,
                                     const std::vector<VkSurfaceFormatKHR>& desiredFormats,
                                     VkFormatFeatureFlags featureFlags);

VkPresentModeKHR FindPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes,
                                 const std::vector<VkPresentModeKHR>& desiredPresentModes);

VkExtent2D FindExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D actualExtent);
struct Swapchain {
  VkDevice device                            = VK_NULL_HANDLE;
  VkSwapchainKHR swapchain                   = VK_NULL_HANDLE;
  uint32_t imageCount                        = 0;
  VkFormat imageFormat                       = VK_FORMAT_UNDEFINED;
  VkColorSpaceKHR colorSpace                 = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  VkExtent2D extent2D                        = {0, 0};
  uint32_t minImageCount                     = 0;
  VkPresentModeKHR presentMode               = VK_PRESENT_MODE_IMMEDIATE_KHR;
  VkAllocationCallbacks* allocationCallbacks = VK_NULL_HANDLE;

  const std::vector<VkImage>& GetImages() const { return images; };
  std::vector<VkImageView> GetImageViews();
  std::vector<VkImageView> GetImageViews(const void* pNext);
  VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
  void DestroyImageViews(const std::vector<VkImageView>& imageViews) const;
  void DestroyImageView(const VkImageView& imageViews) const;
  operator VkSwapchainKHR() const { return swapchain; }
private:
  struct InternalTable{
    PFN_vkGetSwapchainImagesKHR fp_vkGetSwapchainImagesKHR = nullptr;
    PFN_vkCreateImageView fp_vkCreateImageView             = nullptr;
    PFN_vkDestroyImageView fp_vkDestroyImageView           = nullptr;
    PFN_vkDestroySwapchainKHR fp_vkDestroySwapchainKHR     = nullptr;
  } internalTable;
  std::vector<VkImage> images;
  friend class SwapchainBuilder;
  friend void DestroySwapchain(const Swapchain& swapchain);
};

void DestroySwapchain(const Swapchain& swapchain);

using SwapchainBuilderRef = SwapchainBuilder&;
class SwapchainBuilder {
public:
  explicit SwapchainBuilder(const Device& device);
  explicit SwapchainBuilder(const Device& device, const VkSurfaceKHR surface);
  explicit SwapchainBuilder(const VkPhysicalDevice physicalDevice, const VkDevice device,
                            const VkSurfaceKHR surface,
                            uint32_t graphicsQueueIndex = QUEUE_INDEX_MAX_VALUE,
                            uint32_t presentQueueIndex  = QUEUE_INDEX_MAX_VALUE);

  Swapchain Build();
  SwapchainBuilderRef SetOldSwapchain(VkSwapchainKHR oldSwapchain);
  SwapchainBuilderRef SetOldSwapchain(const Swapchain& oldSwapchain);
  SwapchainBuilderRef SetExtent(uint32_t width, uint32_t height);
  SwapchainBuilderRef AddDesiredFormat(VkSurfaceFormatKHR format);
  // Use the default swapchain formats. This is done if no formats are provided.
  // Default surface format is {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
  SwapchainBuilderRef UseDefaultFormat();
  SwapchainBuilderRef SetDesiredPresentMode(VkPresentModeKHR presentMode);
  SwapchainBuilderRef UseDefaultPresentMode();

  SwapchainBuilderRef SetImageUseFlags(VkImageUsageFlags flags);
  SwapchainBuilderRef AddImageUseFlags(VkImageUsageFlags flags);
  // Use the default image usage bitmask values. This is the default if no image usages
  // are provided. The default is VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
  SwapchainBuilderRef UseDefaultImageUseFlags();

  SwapchainBuilderRef SetFormatFeatureFlags(VkFormatFeatureFlags flags);
  SwapchainBuilderRef AddFormatFeatureFlags(VkFormatFeatureFlags flags);
  SwapchainBuilderRef UseDefaultFormatFeatureFlags();

  SwapchainBuilderRef SetImageArrayLayerCount(uint32_t count);

  SwapchainBuilderRef SetMinImageCount(uint32_t count);
  SwapchainBuilderRef SetClipped(bool clipped);

  // Set the VkSwapchainCreateFlagBitsKHR.
  SwapchainBuilder& SetCreateFlags(VkSwapchainCreateFlagBitsKHR flags);
  // Set the transform to be applied, like a 90 degrees rotation. Default is no transform.
  SwapchainBuilder& SetPreTransformFlags(VkSurfaceTransformFlagBitsKHR pre_transform_flags);
  // Set the alpha channel to be used with other windows in on the system. Default is VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
  SwapchainBuilder& SetCompositeAlphaFlags(VkCompositeAlphaFlagBitsKHR flags);

  // Add a structure to the pNext chain of VkSwapchainCreateInfoKHR.
  // The structure must be valid when SwapchainBuilder::build() is called.
  template <typename T>
  SwapchainBuilder& AddPNext(T* structure) {
    info.pNextChain.push_back(reinterpret_cast<VkBaseOutStructure*>(structure));
    return *this;
  }

  // Provide custom allocation callbacks.
  SwapchainBuilder& SetAllocationCallbacks(VkAllocationCallbacks* callbacks);
  enum BufferMode {
    SINGLE_BUFFERING = 1,
    DOUBLE_BUFFERING = 2,
    TRIPLE_BUFFERING = 3,
  };

private:
  void AddDefaultFormats();
  void AddDefaultPresentModes();
  struct SwapchainInfo {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device                 = VK_NULL_HANDLE;
    std::vector<VkBaseOutStructure*> pNextChain;
    VkSwapchainCreateFlagBitsKHR createFlags = static_cast<VkSwapchainCreateFlagBitsKHR>(0);
    VkSurfaceKHR surface                     = VK_NULL_HANDLE;
    std::vector<VkSurfaceFormatKHR> desiredFormats;
    VkExtent2D extent2D                        = {800, 600};
    uint32_t arrayLayerCount                   = 1;
    uint32_t minImageCount                     = 0;
    VkImageUsageFlags imageUsageFlags          = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkFormatFeatureFlags formatFeatureFlags    = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    uint32_t graphicsQueueIndex                = 0;
    uint32_t presentQueueIndex                 = 0;
    VkSurfaceTransformFlagBitsKHR preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(0);
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    std::vector<VkPresentModeKHR> desiredPresentModes;
    bool clipped                               = true;
    VkSwapchainKHR oldSwapchain                = VK_NULL_HANDLE;
    VkAllocationCallbacks* allocationCallbacks = VK_NULL_HANDLE;
  } info;
};
}  // namespace vkh
#endif  //VK_SWAPCHAIN_HPP
