#ifndef VK_FUNCTIONS_HPP
#define VK_FUNCTIONS_HPP

#include <vulkan/vulkan.h>
#include <mutex>

namespace vkh {
class VulkanFunction {
public:
  static VulkanFunction& GetInstance() {
    static VulkanFunction vulkanFunction;
    return vulkanFunction;
  }
  bool InitVulkanFuncs(PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr);
  void InitInstanceFuncs(VkInstance instance);
  template <typename T>
  void GetInstanceProcAddr(T& out_ptr, const char* func_name) {
    out_ptr = reinterpret_cast<T>(fp_vkGetInstanceProcAddr(instance, func_name));
  }

  template <typename T>
  void GetDeviceProcAddr(VkDevice device, T& out_ptr, const char* func_name) {
    out_ptr = reinterpret_cast<T>(fp_vkGetDeviceProcAddr(device, func_name));
  }
  PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr                                   = nullptr;
  PFN_vkEnumerateInstanceExtensionProperties fp_vkEnumerateInstanceExtensionProperties = nullptr;
  PFN_vkEnumerateInstanceLayerProperties fp_vkEnumerateInstanceLayerProperties         = nullptr;
  PFN_vkEnumerateInstanceVersion fp_vkEnumerateInstanceVersion                         = nullptr;
  PFN_vkCreateInstance fp_vkCreateInstance                                             = nullptr;
  PFN_vkDestroyInstance fp_vkDestroyInstance                                           = nullptr;

  PFN_vkEnumeratePhysicalDevices fp_vkEnumeratePhysicalDevices                   = nullptr;
  PFN_vkGetPhysicalDeviceFeatures fp_vkGetPhysicalDeviceFeatures                 = nullptr;
  PFN_vkGetPhysicalDeviceFeatures2 fp_vkGetPhysicalDeviceFeatures2               = nullptr;
  PFN_vkGetPhysicalDeviceFeatures2KHR fp_vkGetPhysicalDeviceFeatures2KHR         = nullptr;
  PFN_vkGetPhysicalDeviceFormatProperties fp_vkGetPhysicalDeviceFormatProperties = nullptr;
  PFN_vkGetPhysicalDeviceImageFormatProperties fp_vkGetPhysicalDeviceImageFormatProperties =
      nullptr;
  PFN_vkGetPhysicalDeviceProperties fp_vkGetPhysicalDeviceProperties   = nullptr;
  PFN_vkGetPhysicalDeviceProperties2 fp_vkGetPhysicalDeviceProperties2 = nullptr;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties fp_vkGetPhysicalDeviceQueueFamilyProperties =
      nullptr;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties2 fp_vkGetPhysicalDeviceQueueFamilyProperties2 =
      nullptr;
  PFN_vkGetPhysicalDeviceMemoryProperties fp_vkGetPhysicalDeviceMemoryProperties   = nullptr;
  PFN_vkGetPhysicalDeviceFormatProperties2 fp_vkGetPhysicalDeviceFormatProperties2 = nullptr;
  PFN_vkGetPhysicalDeviceMemoryProperties2 fp_vkGetPhysicalDeviceMemoryProperties2 = nullptr;

  PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr                                   = nullptr;
  PFN_vkCreateDevice fp_vkCreateDevice                                             = nullptr;
  PFN_vkEnumerateDeviceExtensionProperties fp_vkEnumerateDeviceExtensionProperties = nullptr;

  PFN_vkDestroySurfaceKHR fp_vkDestroySurfaceKHR                                   = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR fp_vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fp_vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fp_vkGetPhysicalDeviceSurfacePresentModesKHR =
      nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fp_vkGetPhysicalDeviceSurfaceCapabilitiesKHR =
      nullptr;

private:
  VulkanFunction()  = default;
  ~VulkanFunction() = default;
  void InitPreInstanceFuncs();
  std::mutex mutex;
  VkInstance instance = nullptr;
};
}  // namespace vkh

#endif  //VK_FUNCTIONS_HPP
