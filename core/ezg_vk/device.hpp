#ifndef EASYGRAPHICS_DEVICE_HPP
#define EASYGRAPHICS_DEVICE_HPP
//#define VK_NO_PROTOTYPES
//#include <vk_mem_alloc.h>
//#include <volk.h>
#include "vulkan_base.hpp"
#include "context.hpp"

namespace ezg::vk {
class Device {
public:
  ~Device();

  void SetContext(const Context& ctx);

  [[nodiscard]] VkInstance Instance() const { return m_instance; }
  [[nodiscard]] VkDevice Handle() const { return m_device; }
  [[nodiscard]] VkPhysicalDevice GPU() const { return m_gpu; }

  void InitVMA();

  void DisplayInfo();

  VkQueue PresentQueue() const { return m_presentQueue; }
  VkQueue GraphicsQueue() const { return m_customQInfo.queues[QUEUE_INDEX_GRAPHICS]; }
  VkQueue ComputeQueue() const { return m_customQInfo.queues[QUEUE_INDEX_COMPUTE]; }
  VkQueue TransferQueue() const { return m_customQInfo.queues[QUEUE_INDEX_TRANSFER]; }
  uint32_t GraphicsQFIndex() const { return m_customQInfo.familyIndices[QUEUE_INDEX_GRAPHICS]; }

  VmaAllocator Allocator() const;

private:
  VkInstance m_instance{VK_NULL_HANDLE};
  VkPhysicalDevice m_gpu{VK_NULL_HANDLE};
  VkDevice m_device{VK_NULL_HANDLE};
  CustomQueueInfo m_customQInfo{};
  VkPhysicalDeviceMemoryProperties m_gpuMemProps{};
  VkPhysicalDeviceProperties m_gpuProps{};
  DeviceFeatures m_gpuFeats{};
  VkQueue m_presentQueue{VK_NULL_HANDLE};
  VmaAllocator m_allocator{VK_NULL_HANDLE};
};
}  // namespace ezg::vk
#endif  //EASYGRAPHICS_DEVICE_HPP
