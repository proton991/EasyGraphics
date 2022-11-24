#ifndef EASYGRAPHICS_DEVICE_HPP
#define EASYGRAPHICS_DEVICE_HPP
#include <volk.h>
#include "context.hpp"


namespace ezg::vk {

class Device {
public:
  void SetContext(const Context& ctx);

  [[nodiscard]] VkDevice Handle() const { return m_device; }

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
  VkPhysicalDevice m_gpu{VK_NULL_HANDLE};
  VkDevice m_device{VK_NULL_HANDLE};
  CustomQueueInfo m_customQInfo{};
  VkPhysicalDeviceMemoryProperties m_gpuMemProps{};
  VkPhysicalDeviceProperties m_gpuProps{};
  DeviceFeatures m_gpuFeats{};
};
}
#endif  //EASYGRAPHICS_DEVICE_HPP
