#ifndef TYPES_HPP
#define TYPES_HPP
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace ege {
struct AllocatedBuffer {
  VkBuffer m_buffer;
  VmaAllocation m_allocation;
};

struct AllocatedImage {
  VkImage m_image;
  VmaAllocation m_allocation;
};
}  // namespace ege
#endif  //TYPES_HPP
