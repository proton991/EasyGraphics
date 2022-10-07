#ifndef __VK_INSTANCE_H__
#define __VK_INSTANCE_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "sys_info.hpp"
#include "vk_tools.hpp"

namespace vkh {

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
  bool enableValidation;
  bool headless;
  // VkInstanceCreateInfo
  std::vector<const char*> layers;
  std::vector<const char*> extensions;
};

struct Instance {
  VkInstance instance                                = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugMessenger            = VK_NULL_HANDLE;
  VkAllocationCallbacks* allocationCallbacks         = VK_NULL_HANDLE;
  PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr = nullptr;
  PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr     = nullptr;

  std::vector<const char*> enabledExtensions;
  std::vector<const char*> enabledLayers;

  // A conversion function which allows this Instance to be used
  // in places where VkInstance would have been used.
  operator VkInstance() const { return this->instance; }

private:
  bool headless       = false;
  uint32_t apiVersion = VK_API_VERSION_1_0;

  friend class InstanceBuilder;
};

void DestroyInstance(Instance instance);

}  // namespace vkh

#endif  // __VK_INSTANCE_H__