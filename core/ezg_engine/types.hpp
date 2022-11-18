#ifndef TYPES_HPP
#define TYPES_HPP
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "vulkan_helper/vk_descriptors.hpp"

namespace ezg {
using ID_TYPE = uint32_t;

struct AllocatedBuffer {
  VkBuffer m_buffer{};
  VmaAllocation m_allocation{};
  VkDeviceSize m_size{0};

  VkDescriptorBufferInfo GetDescriptorBufferInfo(VkDeviceSize offset = 0) const;
};

inline VkDescriptorBufferInfo AllocatedBuffer::GetDescriptorBufferInfo(VkDeviceSize offset) const {
  VkDescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = m_buffer;
  bufferInfo.range  = m_size;
  bufferInfo.offset = offset;

  return bufferInfo;
}

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
  VkDescriptorSet globalDescriptor;

  AllocatedBuffer objectBuffer;
  VkDescriptorSet objectDescriptor;

  vkh::DescriptorAllocator* descriptorAllocator;
};

struct GPUCameraData {
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 viewProj;
};

struct GPUSceneData {
  glm::vec4 fogColor;      // w is for exponent
  glm::vec4 fogDistances;  //x for min, y for max, zw unused.
  glm::vec4 ambientColor;
  glm::vec4 sunlightDirection;  //w for sun power
  glm::vec4 sunlightColor;
};

struct GPUObjectData {
  glm::mat4 modelMatrix;
};

struct UploadContext {
  VkFence uploadFence;
  VkCommandPool cmdPool;
};

struct Texture {
  AllocatedImage image;
  VkImageView imageView;
};

}  // namespace ezg
#endif  //TYPES_HPP
