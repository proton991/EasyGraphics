#ifndef __VK_TOOLS_H__
#define __VK_TOOLS_H__

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <xstring>

#define VULKAN_NON_COPIABLE(ClassName)             \
  ClassName(const ClassName&) = delete;            \
  ClassName(ClassName&&) = delete;                 \
  ClassName& operator=(const ClassName&) = delete; \
  ClassName& operator=(ClassName&&) = delete;

#define VULKAN_HANDLE(VulkanHandleType, name)      \
public:                                            \
  VulkanHandleType Handle() const { return name; } \
                                                   \
private:                                           \
  VulkanHandleType name{};

namespace vkh {

void VkCheck(VkResult result, const char* operation);

const char* ToString(VkResult result);

bool CheckExtensionSupported(std::vector<VkExtensionProperties> const& availableExtensions,
                             const char* extensionName);

bool CheckExtensionsSupported(std::vector<VkExtensionProperties> const& availableExtensions,
                              std::vector<const char*> const& extensionNames);

bool CheckLayerSupported(std::vector<VkLayerProperties> const& availableLayers,
                         const char* layerName);

bool CheckLayersSupported(std::vector<VkLayerProperties> const& availableLayers,
                          std::vector<const char*> const& layerNames);

template <typename T, typename F, typename... Ts>
inline auto GetVector(F&& f, Ts&&... ts) -> std::vector<T> {
  uint32_t count = 0;
  std::vector<T> results;
  f(ts..., &count, nullptr);
  results.resize(count);
  f(ts..., &count, results.data());
  results.resize(count);
  return results;
}

inline void Log(const std::string_view& msg) {
  std::cout << msg << "\n";
}

template<typename T>
inline void Log(const std::vector<T>& vec) {
  for (const auto& item : vec) {
    std::cout << item << "\n";
  }
}
}  // namespace vkh

#endif  // __VK_TOOLS_H__