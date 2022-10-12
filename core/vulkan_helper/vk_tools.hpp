#ifndef __VK_TOOLS_H__
#define __VK_TOOLS_H__

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <xstring>

#define VULKAN_NON_COPIABLE(ClassName)             \
  ClassName(const ClassName&) = delete;            \
  ClassName(ClassName&&)      = delete;            \
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

template <typename T>
inline void Log(const std::vector<T>& vec) {
  for (const auto& item : vec) {
    std::cout << item << "\n";
  }
}

template <typename T>
void SetPNextChain(T& structure, const std::vector<VkBaseOutStructure*>& structs) {
  structure.pNext = nullptr;
  if (structs.empty())
    return;
  for (uint32_t i = 0; i < structs.size() - 1; i++) {
    structs.at(i)->pNext = structs.at(i + 1);
  }
  structure.pNext = structs.at(0);
}

enum class Severity {
  Verbos  = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
  Info    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
  Warning = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
  Error   = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
  Fatal   = VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT
};

class Console final {
public:
  template <class Action>
  static void Write(const Severity severity, const Action action) {
    const auto attributes = SetColorBySeverity(severity);
    action();
    SetColorByAttributes(attributes);
  }

  static int SetColorBySeverity(Severity severity) noexcept;
  static void SetColorByAttributes(int attributes) noexcept;
};

inline const char* ObjectTypeToString(const VkObjectType objectType) {
  switch (objectType) {
#define STR(e)             \
  case VK_OBJECT_TYPE_##e: \
    return #e
    STR(UNKNOWN);
    STR(INSTANCE);
    STR(PHYSICAL_DEVICE);
    STR(DEVICE);
    STR(QUEUE);
    STR(SEMAPHORE);
    STR(COMMAND_BUFFER);
    STR(FENCE);
    STR(DEVICE_MEMORY);
    STR(BUFFER);
    STR(IMAGE);
    STR(EVENT);
    STR(QUERY_POOL);
    STR(BUFFER_VIEW);
    STR(IMAGE_VIEW);
    STR(SHADER_MODULE);
    STR(PIPELINE_CACHE);
    STR(PIPELINE_LAYOUT);
    STR(RENDER_PASS);
    STR(PIPELINE);
    STR(DESCRIPTOR_SET_LAYOUT);
    STR(SAMPLER);
    STR(DESCRIPTOR_POOL);
    STR(DESCRIPTOR_SET);
    STR(FRAMEBUFFER);
    STR(COMMAND_POOL);
    STR(SAMPLER_YCBCR_CONVERSION);
    STR(DESCRIPTOR_UPDATE_TEMPLATE);
    STR(SURFACE_KHR);
    STR(SWAPCHAIN_KHR);
    STR(DISPLAY_KHR);
    STR(DISPLAY_MODE_KHR);
    STR(DEBUG_REPORT_CALLBACK_EXT);
    STR(DEBUG_UTILS_MESSENGER_EXT);
    STR(ACCELERATION_STRUCTURE_KHR);
    STR(VALIDATION_CACHE_EXT);
    STR(PERFORMANCE_CONFIGURATION_INTEL);
    STR(DEFERRED_OPERATION_KHR);
    STR(INDIRECT_COMMANDS_LAYOUT_NV);
#undef STR
    default:
      return "unknown";
  }
}


}  // namespace vkh

#endif  // __VK_TOOLS_H__