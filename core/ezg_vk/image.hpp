#ifndef EASYGRAPHICS_IMAGE_HPP
#define EASYGRAPHICS_IMAGE_HPP
//#include <volk.h>
//#define VK_NO_PROTOTYPES
//#include <vk_mem_alloc.h>
#include "vulkan_base.hpp"
#include <string>
#include "common.hpp"

namespace ezg::vk {
class Device;
struct ImageCreateInfo {
  VkFormat format;
  VkImageUsageFlags usage;
  VkImageAspectFlags aspect;
  VkSampleCountFlagBits samples;
  VmaAllocationCreateFlags vmaAllocFlags;
  std::string name;
  VkExtent2D imageExtent;
};
class Image {
public:
  NO_COPY(Image)
  NO_MOVE(Image)

  Image(const Device& device, const ImageCreateInfo& info);
  ~Image();

  [[nodiscard]] VkImage Handle() const { return m_image; }

  [[nodiscard]] VkImageView GetView() const { return m_view; }

private:
  const Device& m_device;
  VkImage m_image{VK_NULL_HANDLE};
  VkImageView m_view{VK_NULL_HANDLE};
  VmaAllocation m_allocation{VK_NULL_HANDLE};
  VkFormat m_format{VK_FORMAT_UNDEFINED};
  std::string m_name;
};
}
#endif  //EASYGRAPHICS_IMAGE_HPP
