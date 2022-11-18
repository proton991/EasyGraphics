#ifndef EASYGRAPHICS_CONTEXT_HPP
#define EASYGRAPHICS_CONTEXT_HPP

#include <vulkan/vulkan_core.h>
#include <memory>

namespace ezg::vk {

class Instance {
public:
  [[nodiscard]] VkInstance Handle() const { return m_instance; };
private:
  VkInstance m_instance{VK_NULL_HANDLE};

};

class Device {
public:
private:
  VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
  VkDevice m_device{VK_NULL_HANDLE};
};

class Context {
public:
  void InitLoader();

private:
  std::shared_ptr<Instance> m_instance;
  std::shared_ptr<Device> m_device;

};

}  // namespace ezg::vk
#endif  //EASYGRAPHICS_CONTEXT_HPP
