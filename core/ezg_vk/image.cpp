#include "image.hpp"
#include <utility>
#include "debug.hpp"
#include "device.hpp"

namespace ezg::vk {
Image::Image(const Device& device, const ImageCreateInfo& info)
    : m_device(device), m_format(info.format), m_name(info.name) {
  VkImageCreateInfo imageInfo = {
      .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format    = m_format,
      .extent{
          .width  = info.imageExtent.width,
          .height = info.imageExtent.height,
          .depth  = 1,
      },
      .mipLevels     = 1,
      .arrayLayers   = 1,
      .samples       = info.samples,
      .tiling        = VK_IMAGE_TILING_OPTIMAL,
      .usage         = info.usage,
      .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  const VmaAllocationCreateInfo vmaAllocInfo{
      .flags = info.vmaAllocFlags,
      .usage = VMA_MEMORY_USAGE_AUTO,
  };

  Check(vmaCreateImage(m_device.Allocator(), &imageInfo, &vmaAllocInfo, &m_image, &m_allocation,
                       nullptr),
        "vma create image");
    vmaSetAllocationName(m_device.Allocator(), m_allocation, m_name.c_str());

  // Assign an internal name using Vulkan debug markers.
  DebugUtil::Get().SetObjectName(m_image, m_name.c_str());

  VkImageViewCreateInfo imageViewInfo = {.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                         .image    = m_image,
                                         .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                         .format   = m_format,
                                         .subresourceRange{
                                             .aspectMask     = info.aspect,
                                             .baseMipLevel   = 0,
                                             .levelCount     = 1,
                                             .baseArrayLayer = 0,
                                             .layerCount     = 1,
                                         }};
  vkCreateImageView(m_device.Handle(), &imageViewInfo, nullptr, &m_view);
}
Image::~Image() {
  vkDestroyImageView(m_device.Handle(), m_view, nullptr);
  vmaDestroyImage(m_device.Allocator(), m_image, m_allocation);
}
}  // namespace ezg::vk