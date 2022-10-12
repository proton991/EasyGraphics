#include "vk_functions.hpp"
#include "vk_library.hpp"
namespace vkh {

bool VulkanFunction::InitVulkanFuncs(PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr) {
  std::lock_guard<std::mutex> lockGuard(mutex);
  if (fp_vkGetInstanceProcAddr != nullptr) {
    this->fp_vkGetInstanceProcAddr = fp_vkGetInstanceProcAddr;
  } else {
    auto& lib                      = VulkanLibrary::GetInstance();
    this->fp_vkGetInstanceProcAddr = lib.fp_vkGetInstanceProcAddr;
    if (lib.library == nullptr || lib.fp_vkGetInstanceProcAddr == VK_NULL_HANDLE)
      return false;
  }
  InitPreInstanceFuncs();
  return true;
}
void VulkanFunction::InitInstanceFuncs(VkInstance instance) {
  this->instance = instance;

  GetInstanceProcAddr(fp_vkDestroyInstance, "vkDestroyInstance");
  GetInstanceProcAddr(fp_vkEnumeratePhysicalDevices, "vkEnumeratePhysicalDevices");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceFeatures, "vkGetPhysicalDeviceFeatures");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceFeatures2, "vkGetPhysicalDeviceFeatures2");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceFeatures2KHR, "vkGetPhysicalDeviceFeatures2KHR");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceFormatProperties,
                      "vkGetPhysicalDeviceFormatProperties");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceImageFormatProperties,
                      "vkGetPhysicalDeviceImageFormatProperties");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceProperties, "vkGetPhysicalDeviceProperties");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceProperties2, "vkGetPhysicalDeviceProperties2");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceQueueFamilyProperties,
                      "vkGetPhysicalDeviceQueueFamilyProperties");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceQueueFamilyProperties2,
                      "vkGetPhysicalDeviceQueueFamilyProperties2");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceMemoryProperties,
                      "vkGetPhysicalDeviceMemoryProperties");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceFormatProperties2,
                      "vkGetPhysicalDeviceFormatProperties2");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceMemoryProperties2,
                      "vkGetPhysicalDeviceMemoryProperties2");

  GetInstanceProcAddr(fp_vkGetDeviceProcAddr, "vkGetDeviceProcAddr");
  GetInstanceProcAddr(fp_vkCreateDevice, "vkCreateDevice");
  GetInstanceProcAddr(fp_vkEnumerateDeviceExtensionProperties,
                      "vkEnumerateDeviceExtensionProperties");

  GetInstanceProcAddr(fp_vkDestroySurfaceKHR, "vkDestroySurfaceKHR");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceSurfaceSupportKHR,
                      "vkGetPhysicalDeviceSurfaceSupportKHR");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceSurfaceFormatsKHR,
                      "vkGetPhysicalDeviceSurfaceFormatsKHR");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceSurfacePresentModesKHR,
                      "vkGetPhysicalDeviceSurfacePresentModesKHR");
  GetInstanceProcAddr(fp_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
                      "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
}

void VulkanFunction::InitPreInstanceFuncs() {
  fp_vkEnumerateInstanceExtensionProperties =
      reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
          fp_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties"));
  fp_vkEnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
      fp_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceLayerProperties"));
  fp_vkEnumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
      fp_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"));
  fp_vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
      fp_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance"));
}
}  // namespace vkh