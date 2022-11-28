#ifndef EASYGRAPHICS_DEVICE_HPP
#define EASYGRAPHICS_DEVICE_HPP
#include <volk.h>
#include "context.hpp"

namespace ezg::vk {

class Device {
public:
  void SetContext(const Context& ctx);

  [[nodiscard]] VkInstance Instance() const { return m_instance; }
  [[nodiscard]] VkDevice Handle() const { return m_device; }
  [[nodiscard]] VkPhysicalDevice GPU() const { return m_gpu; }
  void DisplayInfo();

  VkQueue GetQueue(QueueIndices qIndex) {
    assert(qIndex < QUEUE_INDEX_COUNT);
    return m_customQInfo.queues[qIndex];
  }

  uint32_t GetQFamilyIndex(QueueIndices qIndex) {
    assert(qIndex < QUEUE_INDEX_COUNT);
    return m_customQInfo.familyIndices[qIndex];
  }

private:
  VkInstance m_instance{VK_NULL_HANDLE};
  VkPhysicalDevice m_gpu{VK_NULL_HANDLE};
  VkDevice m_device{VK_NULL_HANDLE};
  CustomQueueInfo m_customQInfo{};
  VkPhysicalDeviceMemoryProperties m_gpuMemProps{};
  VkPhysicalDeviceProperties m_gpuProps{};
  DeviceFeatures m_gpuFeats{};
};
}  // namespace ezg::vk
#endif  //EASYGRAPHICS_DEVICE_HPP
