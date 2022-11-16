#include "vk_debug.hpp"
#include "vk_functions.hpp"

namespace vkh::debug {
DebugUtil::DebugUtil() {
  VulkanFunction::GetInstance().GetInstanceProcAddr(fp_vkSetDebugUtilsObjectNameEXT, "vkSetDebugUtilsObjectNameEXT");
  if (fp_vkSetDebugUtilsObjectNameEXT == nullptr) {
    throw std::runtime_error("failed to get address of 'vkSetDebugUtilsObjectNameEXT'");
  }
}

VkResult CreateDebugUtilsMessenger(VkInstance instance,
                                   PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
                                   VkDebugUtilsMessageSeverityFlagsEXT severity,
                                   VkDebugUtilsMessageTypeFlagsEXT type, void* user_data_pointer,
                                   VkDebugUtilsMessengerEXT* pDebugMessenger,
                                   VkAllocationCallbacks* allocation_callbacks) {
  switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      break;
    default:
      throw std::invalid_argument("invalid threshold");
  }
  VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
  messengerCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messengerCreateInfo.pNext           = nullptr;
  messengerCreateInfo.messageSeverity = severity;
  messengerCreateInfo.messageType     = type;
  messengerCreateInfo.pfnUserCallback = debug_callback;
  messengerCreateInfo.pUserData       = user_data_pointer;

  PFN_vkCreateDebugUtilsMessengerEXT createMessengerFunc;
  VulkanFunction::GetInstance().GetInstanceProcAddr(createMessengerFunc,
                                                    "vkCreateDebugUtilsMessengerEXT");

  if (createMessengerFunc != nullptr) {
    return createMessengerFunc(instance, &messengerCreateInfo, allocation_callbacks,
                               pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

}  // namespace vkh::debug