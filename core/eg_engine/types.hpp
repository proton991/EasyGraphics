#ifndef TYPES_HPP
#define TYPES_HPP
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace ege {
using ID_TYPE = uint32_t;

struct AllocatedBuffer {
  VkBuffer m_buffer;
  VmaAllocation m_allocation;
};

struct AllocatedImage {
  VkImage m_image;
  VmaAllocation m_allocation;
};

struct MeshPushConstants {
  glm::vec4 data;
  glm::mat4 renderMatrix;
};
}  // namespace ege
#endif  //TYPES_HPP
