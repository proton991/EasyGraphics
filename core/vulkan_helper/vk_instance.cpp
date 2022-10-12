#include "vk_instance.hpp"
#include <cassert>
#include "vk_debug.hpp"
#include <stdexcept>
#include "vk_functions.hpp"

namespace vkh {
InstanceBuilder::InstanceBuilder() {
  // Initiate application info with default value
  appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.engineVersion      = VK_MAKE_API_VERSION(0, 1, 0, 0);
  appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  appInfo.pNext              = nullptr;
  appInfo.pApplicationName   = "Easy Graphics With Vulkan";
  appInfo.pEngineName        = "No Engine";
  appInfo.apiVersion         = VK_API_VERSION_1_3;

  // Initiate instance info with default value
  instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

  // Enbale validation by default
  enableValidation = true;

  headless = false;
}

Instance InstanceBuilder::Build() {
  if (enableValidation && (!SystemInfo::GetInstance().validationLayerAvailable)) {
    throw std::runtime_error("Validation layer required but not supported!");
  }
  if (SystemInfo::GetInstance().debugUtilsAvailable) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  if (!headless) {
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back("VK_KHR_win32_surface");
    // TODO: add cross-platform surface support
  }
  bool allExtensionSupported =
      CheckExtensionsSupported(SystemInfo::GetInstance().availableExtensions, extensions);
  if (!allExtensionSupported) {
    throw std::runtime_error("Required extension not supported!");
  }
  if (enableValidation && SystemInfo::GetInstance().validationLayerAvailable) {
    layers.push_back("VK_LAYER_KHRONOS_validation");
  }

  bool allLayersSupported = CheckLayersSupported(SystemInfo::GetInstance().availableLayers, layers);
  if (!allLayersSupported) {
    throw std::runtime_error("Required layer not supported!");
  }

  // must be placed outside the if block to ensure that it is not destroyed before vkCreateInstance call
  VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
  std::vector<VkBaseOutStructure*> pNextChain;
  if (enableValidation) {
    messengerCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.pNext           = nullptr;
    messengerCreateInfo.pfnUserCallback = dumInfo.debugCallback;
    messengerCreateInfo.messageType     = dumInfo.debugMessageType;
    messengerCreateInfo.messageSeverity = dumInfo.debugMessageSeverity;
    messengerCreateInfo.pUserData       = dumInfo.debugUserDataPointer;
    pNextChain.push_back(reinterpret_cast<VkBaseOutStructure*>(&messengerCreateInfo));
  }
  SetPNextChain(instanceInfo, pNextChain);
#if !defined(NDEBUG)
  for (auto& node : pNextChain) {
    assert(node->sType != VK_STRUCTURE_TYPE_APPLICATION_INFO);
  }
#endif
  Instance instance;
  instance.enabledExtensions.insert(instance.enabledExtensions.end(), extensions.begin(),
                                    extensions.end());
  instance.enabledLayers.insert(instance.enabledLayers.end(), layers.begin(), layers.end());

  instanceInfo.flags                   = static_cast<VkInstanceCreateFlags>(0);
  instanceInfo.pApplicationInfo        = &appInfo;
  instanceInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
  instanceInfo.ppEnabledExtensionNames = extensions.data();
  instanceInfo.enabledLayerCount       = static_cast<uint32_t>(layers.size());
  instanceInfo.ppEnabledLayerNames     = layers.data();
  VkCheck(
      VulkanFunction::GetInstance().fp_vkCreateInstance(&instanceInfo, nullptr, &instance.instance),
      "Create instance");
  VulkanFunction::GetInstance().InitInstanceFuncs(instance.instance);

  VkCheck(debug::CreateDebugUtilsMessenger(instance.instance, dumInfo.debugCallback,
                                           dumInfo.debugMessageSeverity, dumInfo.debugMessageType,
                                           dumInfo.debugUserDataPointer, &instance.debugMessenger,
                                           instance.allocationCallbacks),
          "Create debug utils messenger");
  instance.apiVersion               = appInfo.apiVersion;
  instance.fp_vkGetInstanceProcAddr = VulkanFunction::GetInstance().fp_vkGetInstanceProcAddr;
  instance.fp_vkGetDeviceProcAddr   = VulkanFunction::GetInstance().fp_vkGetDeviceProcAddr;
  instance.enableValidation         = enableValidation;
  return instance;
}

InstanceBuilderRef InstanceBuilder::SetAppName(const char* name) {
  appInfo.pApplicationName = name;
  return *this;
}

InstanceBuilderRef InstanceBuilder::SetEngineName(const char* name) {
  appInfo.pEngineName = name;
  return *this;
}

InstanceBuilderRef InstanceBuilder::SetEngineVersion(uint32_t major, uint32_t minor,
                                                     uint32_t patch) {
  appInfo.engineVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
  return *this;
}

InstanceBuilderRef InstanceBuilder::SetApiVersion(uint32_t major, uint32_t minor, uint32_t patch) {
  appInfo.apiVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
  return *this;
}

InstanceBuilderRef InstanceBuilder::EnableValidationLayers(bool enable) {
  enableValidation = true;
  return *this;
}

InstanceBuilderRef InstanceBuilder::EnableExtension(const char* name) {
  if (name) {
    extensions.push_back(name);
  }
  return *this;
}

InstanceBuilderRef InstanceBuilder::EnableLayer(const char* name) {
  if (name) {
    layers.push_back(name);
  }
  return *this;
}

}  // namespace vkh
