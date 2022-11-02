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

struct FrameData {
  VkSemaphore presentSemaphore;
  VkSemaphore renderSemaphore;

  VkFence renderFence;
  VkCommandPool cmdPool;
  VkCommandBuffer cmdBuffer;

  AllocatedBuffer cameraBuffer;
};

struct GPUCameraData{
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 viewProj;
};

}  // namespace ege
#endif  //TYPES_HPP
