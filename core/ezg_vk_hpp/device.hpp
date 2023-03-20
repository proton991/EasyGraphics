#ifndef EASYGRAPHICS_DEVICE_HPP
#define EASYGRAPHICS_DEVICE_HPP
#include <vk_mem_alloc.h>
#include "context.hpp"

namespace ezg::vulkan {
class Device {
public:
  Device() = default;
  ~Device();
  void SetContext(Context& ctx);

  VkQueue GraphicsQueue() const { return m_queue_info.queues[QUEUE_INDEX_GRAPHICS]; }
  VkQueue ComputeQueue() const { return m_queue_info.queues[QUEUE_INDEX_COMPUTE]; }
  VkQueue TransferQueue() const { return m_queue_info.queues[QUEUE_INDEX_TRANSFER]; }

  VmaAllocator Allocator() const;

private:
  void initVMA();
  void displayInfo();

  vk::UniqueInstance m_instance{nullptr};
  vk::PhysicalDevice m_gpu{nullptr};
  vk::UniqueDevice m_device{nullptr};
  QueueInfo m_queue_info{};
  vk::PhysicalDeviceMemoryProperties m_mem_props{};
  vk::PhysicalDeviceProperties m_gpu_props{};
  DeviceFeatures m_features;
  VmaAllocator m_allocator{VK_NULL_HANDLE};
};
}  // namespace ezg::vulkan
#endif  //EASYGRAPHICS_DEVICE_HPP
