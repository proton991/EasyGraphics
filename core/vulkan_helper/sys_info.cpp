#include <iostream>
#include "sys_info.hpp"
#include "vk_functions.hpp"
#include "vk_tools.hpp"
namespace vkh {
const char* VK_VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";
SystemInfo::SystemInfo() {
  // Initiate vulkan
  if (!VulkanFunction::GetInstance().InitVulkanFuncs(nullptr)) {
    throw std::runtime_error("Vulkan in not available!");
  }
  // Get instance extensions and layers
  GetEnumerateVector(static_cast<const char*>(nullptr),
                     VulkanFunction::GetInstance().fp_vkEnumerateInstanceExtensionProperties,
                     availableExtensions);
  GetEnumerateVector(VulkanFunction::GetInstance().fp_vkEnumerateInstanceLayerProperties,
                     availableLayers);

  // Check for validation layer and debug extensions
  for (const auto& layer : availableLayers) {
    if (strcmp(layer.layerName, VK_VALIDATION_LAYER_NAME) == 0) {
      validationLayerAvailable = true;
    }
  }
  for (const auto& ext : availableExtensions) {
    if (strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
      debugUtilsAvailable = true;
    }
  }

  for (const auto& layer : availableLayers) {
    std::vector<VkExtensionProperties> layerExtensions;
    GetEnumerateVector(layer.layerName,
                       VulkanFunction::GetInstance().fp_vkEnumerateInstanceExtensionProperties,
                       layerExtensions);
    availableExtensions.insert(availableExtensions.end(), layerExtensions.begin(),
                               layerExtensions.end());
    for (const auto& ext : layerExtensions) {
      if (strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
        debugUtilsAvailable = true;
      }
    }
  }
}

SystemInfo& SystemInfo::GetInstance() {
  static SystemInfo systemInfo;
  return systemInfo;
}
}  // namespace vkh
