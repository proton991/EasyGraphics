#include "vk_destructor.hpp"
#include "vk_device.hpp"
#include "vk_functions.hpp"

namespace vkh {

void DestroyInstance(Instance instance) {
  if (instance.instance != VK_NULL_HANDLE) {
    VulkanFunction::GetInstance().fp_vkDestroyInstance(instance.instance,
                                                       instance.allocationCallbacks);
  }
}

void DestroyDevice(Device device) {
  device.internalTable.fp_vkDestroyDevice(device.vkDevice, device.allocation_callbacks);
}

void DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                VkAllocationCallbacks* allocation_callbacks) {

  PFN_vkDestroyDebugUtilsMessengerEXT deleteMessengerFunc;
  VulkanFunction::GetInstance().GetInstanceProcAddr(deleteMessengerFunc,
                                                    "vkDestroyDebugUtilsMessengerEXT");

  if (deleteMessengerFunc != nullptr) {
    deleteMessengerFunc(instance, debugMessenger, allocation_callbacks);
  }
}
}  // namespace vkh