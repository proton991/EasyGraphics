#ifndef __VK_INSTANCE_H__
#define __VK_INSTANCE_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "sys_info.hpp"
#include "vk_tools.hpp"

namespace vkh {

inline VkBool32 VKAPI_CALL VulkanDebugCallback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    const VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* const pCallbackData, void* const pUserData) {
  (void)pUserData;

  const auto attributes = Console::SetColorBySeverity(static_cast<Severity>(messageSeverity));

  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      std::cerr << "VERBOSE: ";
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      std::cerr << "INFO: ";
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      std::cerr << "WARNING: ";
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      std::cerr << "ERROR: ";
      break;
    default:;
      std::cerr << "UNKNOWN: ";
  }

  switch (messageType) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
      std::cerr << "GENERAL: ";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
      std::cerr << "VALIDATION: ";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
      std::cerr << "PERFORMANCE: ";
      break;
    default:
      std::cerr << "UNKNOWN: ";
  }

  std::cerr << pCallbackData->pMessage;

  if (pCallbackData->objectCount > 0 &&
      messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    std::cerr << "\n\n  Objects (" << pCallbackData->objectCount << "):\n";

    for (uint32_t i = 0; i != pCallbackData->objectCount; ++i) {
      const auto object = pCallbackData->pObjects[i];
      std::cerr << "  - Object[" << i << "]: "
                << "Type: " << ObjectTypeToString(object.objectType) << ", "
                << "Handle: " << reinterpret_cast<void*>(object.objectHandle) << ", "
                << "Name: '" << (object.pObjectName ? object.pObjectName : "") << "'"
                << "\n";
    }
  }

  std::cerr << std::endl;

  Console::SetColorByAttributes(attributes);

  return VK_FALSE;
}

// forward declaration
struct Instance;
class InstanceBuilder;

using InstanceBuilderRef = InstanceBuilder&;
class InstanceBuilder {
public:
  explicit InstanceBuilder();

  Instance Build();

  InstanceBuilderRef SetAppName(const char* name);
  InstanceBuilderRef SetEngineName(const char* name);
  InstanceBuilderRef SetEngineVersion(uint32_t major, uint32_t minor, uint32_t patch);
  InstanceBuilderRef SetApiVersion(uint32_t major, uint32_t minor, uint32_t patch);

  InstanceBuilderRef EnableValidationLayers(bool enable = true);
  InstanceBuilderRef EnableExtension(const char* name);
  InstanceBuilderRef EnableLayer(const char* name);

private:
  VkInstanceCreateInfo instanceInfo{};
  VkApplicationInfo appInfo{};
  struct {
    // debug callback - use the default so it is not nullptr
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback = VulkanDebugCallback;
    VkDebugUtilsMessageSeverityFlagsEXT debugMessageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    VkDebugUtilsMessageTypeFlagsEXT debugMessageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    void* debugUserDataPointer = nullptr;
  } dumInfo;
  bool enableValidation;
  bool headless;
  // VkInstanceCreateInfo
  std::vector<const char*> layers;
  std::vector<const char*> extensions;
};

struct Instance {
  VkInstance instance                                = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugUtilsMessenger       = VK_NULL_HANDLE;
  VkAllocationCallbacks* allocationCallbacks         = VK_NULL_HANDLE;
  PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr = nullptr;
  PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr     = nullptr;

  std::vector<const char*> enabledExtensions;
  std::vector<const char*> enabledLayers;
  bool enableValidation;

  // A conversion function which allows this Instance to be used
  // in places where VkInstance would have been used.
  operator VkInstance() const { return this->instance; }

private:
  bool headless       = false;
  uint32_t apiVersion = VK_API_VERSION_1_0;

  friend class InstanceBuilder;
};

}  // namespace vkh

#endif  // __VK_INSTANCE_H__