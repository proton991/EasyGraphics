#ifndef __SYS_INFO_H__
#define __SYS_INFO_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "vk_tools.hpp"
namespace vkh {

struct SystemInfo {
public:
  static SystemInfo& GetInstance();
  bool validationLayerAvailable;
  bool debugUtilsAvailable;
  std::vector<VkLayerProperties> availableLayers;
  std::vector<VkExtensionProperties> availableExtensions;

  inline void DisplayInfo() {
    Log("Instance available layers are:");
    for (const auto& layer : availableLayers) {
      Log(layer.layerName);
    }
    Log("\nInstance available extensions are:");
    for (const auto& ext : availableExtensions) {
      Log(ext.extensionName);
    }
  }

private:
  VULKAN_NON_COPIABLE(SystemInfo)
  SystemInfo();
  ~SystemInfo() = default;
};

}  // namespace vkh

#endif  // __SYS_INFO_H__