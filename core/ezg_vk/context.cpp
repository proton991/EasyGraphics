#include "context.hpp"
#include <mutex>
#include "spdlog/spdlog.h"

namespace spd = spdlog;
namespace ezg::vk {

static std::mutex mutex;
static PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr{VK_NULL_HANDLE};
static PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr{VK_NULL_HANDLE};
static bool loaderInit = false;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_util_messenger_cb(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  auto* context = static_cast<Context*>(pUserData);

  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        spd::error("[Vulkan]: Validation Error: {}", pCallbackData->pMessage);
      } else
        spd::error("[Vulkan]: Other Error: {}", pCallbackData->pMessage);
      break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        spd::warn("[Vulkan]: Validation Warning: {}", pCallbackData->pMessage);
      else
        spd::warn("[Vulkan]: Other Warning: {}", pCallbackData->pMessage);
      break;

#if 0
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			LOGI("[Vulkan]: Validation Info: %s\n", pCallbackData->pMessage);
		else
			LOGI("[Vulkan]: Other Info: %s\n", pCallbackData->pMessage);
		break;
#endif

    default:
      return VK_FALSE;
  }

  bool log_object_names = false;
  for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
    auto* name = pCallbackData->pObjects[i].pObjectName;
    if (name) {
      log_object_names = true;
      break;
    }
  }

  if (log_object_names) {
    for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
      auto* name = pCallbackData->pObjects[i].pObjectName;
      spd::info("  Object #%u: {}", i, name ? name : "N/A");
    }
  }

  return VK_FALSE;
}

VulkanLib& VulkanLib::Get() {
  static VulkanLib vkLib;
  return vkLib;
}

VulkanLib::~VulkanLib() {
#if defined(__linux__) || defined(__APPLE__)
  dlclose(m_library);
#elif defined(_WIN32)
  FreeLibrary(m_library);
#endif
  m_library = nullptr;
}

VulkanLib::VulkanLib() {
#if defined(__linux__)
  library = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
  if (!library)
    library = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
#elif defined(__APPLE__)
  library = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
  if (!library)
    library = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
#elif defined(_WIN32)
  m_library = LoadLibrary(TEXT("vulkan-1.dll"));
#else
  assert(false && "Unsupported platform");
#endif
  if (!m_library)
    return;
  LoadFunc(fp_vkGetInstanceProcAddr, "vkGetInstanceProcAddr");
}

bool Context::InitLoader(PFN_vkGetInstanceProcAddr addr) {
  if (loaderInit) {
    return true;
  }
  std::lock_guard<std::mutex> lockGuard(mutex);
  if (addr != nullptr) {
    fp_vkGetInstanceProcAddr = addr;
  } else {
    fp_vkGetInstanceProcAddr = VulkanLib::Get().fp_vkGetInstanceProcAddr;
    if (fp_vkGetInstanceProcAddr == VK_NULL_HANDLE) {
      return false;
    }
  }
  loaderInit = true;
  volkInitializeCustom(fp_vkGetInstanceProcAddr);
  return true;
}

PFN_vkGetInstanceProcAddr Context::InstanceFuncLoader() {
  return fp_vkGetInstanceProcAddr;
}

PFN_vkGetDeviceProcAddr Context::DeviceFuncLoader() {
  return fp_vkGetDeviceProcAddr;
}

Context::Context() {
  //query instance layers
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  m_layers.resize(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, m_layers.data());
  spd::info("Layer Count: {}", layerCount);
  for (auto& layer : m_layers) {
    spd::info("Found layer: {}", layer.layerName);
  }
  // query instance extensions
  uint32_t instanceExtCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtCount, nullptr);
  m_instanceExts.resize(instanceExtCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtCount, m_instanceExts.data());
  spd::info("Instance Extension Count: {}", instanceExtCount);
  for (auto& instanceExt : m_instanceExts) {
    spd::info("Found extension: {}", instanceExt.extensionName);
  }
  m_appInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  m_appInfo.apiVersion       = VK_API_VERSION_1_1;
  m_appInfo.engineVersion    = VK_API_VERSION_1_1;
  m_appInfo.pNext            = nullptr;
  m_appInfo.pApplicationName = "Easy Graphics With Vulkan";
  m_appInfo.pEngineName      = "No Engine";
}

Context& Context::EnableInstanceExt(const char* ext) {
  m_requiredInstExts.push_back(ext);
  return *this;
}

Context& Context::EnableInstanceLayer(const char* layer) {
  m_requiredLayers.push_back(layer);
  return *this;
}

bool Context::CreateInstance() {
  const auto has_extension = [&](const char* name) -> bool {
    auto itr = find_if(m_instanceExts.begin(), m_instanceExts.end(),
                       [name](const VkExtensionProperties& e) -> bool {
                         return strcmp(e.extensionName, name) == 0;
                       });
    return itr != m_instanceExts.end();
  };
  const auto has_layer = [&](const char* name) -> bool {
    auto layer_itr =
        find_if(m_layers.begin(), m_layers.end(), [name](const VkLayerProperties& e) -> bool {
          return strcmp(e.layerName, name) == 0;
        });
    return layer_itr != m_layers.end();
  };
  VkInstanceCreateInfo instanceInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  instanceInfo.pApplicationInfo     = &m_appInfo;

  VkValidationFeaturesEXT validationFeatures = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT};
  std::vector<const char*> instanceExts;
  std::vector<const char*> instanceLayers;
  instanceExts.insert(instanceExts.end(), m_requiredInstExts.begin(), m_requiredInstExts.end());
  instanceLayers.insert(instanceLayers.end(), m_requiredLayers.begin(), m_requiredLayers.end());

  if (has_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
    instanceExts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    ext.supports_debug_utils = true;
  }
  instanceExts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(_WIN32)
  instanceExts.push_back("VK_KHR_win32_surface");
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
  instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
  instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
  instanceExtensions.push_back(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
  instanceExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
  instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
  instanceExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
#endif

  if (m_enableValidation && has_layer("VK_LAYER_KHRONOS_validation")) {
    instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
    spd::info("Enabling VK_LAYER_KHRONOS_validation.");

    uint32_t layerExtCount = 0;
    vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerExtCount, nullptr);
    std::vector<VkExtensionProperties> layerExts(layerExtCount);
    vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerExtCount,
                                           layerExts.data());
    if (find_if(layerExts.begin(), layerExts.end(), [](const VkExtensionProperties& e) {
          return strcmp(e.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0;
        }) != layerExts.end()) {
      instanceExts.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
      static const VkValidationFeatureEnableEXT validationSyncFeatures[1] = {
          VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
      };
      spd::info("Enabling VK_EXT_validation_features for synchronization validation.");
      validationFeatures.enabledValidationFeatureCount = 1;
      validationFeatures.pEnabledValidationFeatures    = validationSyncFeatures;

      instanceInfo.pNext = &validationFeatures;
    }

    if (!ext.supports_debug_utils &&
        find_if(layerExts.begin(), layerExts.end(), [](const VkExtensionProperties& e) {
          return strcmp(e.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
        }) != end(layerExts)) {
      instanceExts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      ext.supports_debug_utils = true;
    }
  }

  instanceInfo.enabledExtensionCount   = instanceExts.size();
  instanceInfo.ppEnabledExtensionNames = instanceExts.empty() ? nullptr : instanceExts.data();
  instanceInfo.enabledLayerCount       = instanceLayers.size();
  instanceInfo.ppEnabledLayerNames     = instanceLayers.empty() ? nullptr : instanceLayers.data();

  if (m_instance == VK_NULL_HANDLE) {
    if (vkCreateInstance(&instanceInfo, nullptr, &m_instance) != VK_SUCCESS) {
      return false;
    }
  }
  volkLoadInstance(m_instance);

  for (auto& layer : instanceLayers) {
    spd::info("Enabling instance layer: {}", layer);
  }

  for (auto& instanceExt : instanceExts) {
    spd::info("Enabling instance extension: {}", instanceExt);
  }

  if (ext.supports_debug_utils) {
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debugInfo.pfnUserCallback = debug_util_messenger_cb;
    debugInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    debugInfo.pUserData = this;

    // For some reason, this segfaults Android, sigh ... We get relevant output in logcat anyways.
    if (vkCreateDebugUtilsMessengerEXT)
      vkCreateDebugUtilsMessengerEXT(m_instance, &debugInfo, nullptr, &m_debugMessenger);
  }
  return true;
}

Context::~Context() {
  Destroy();
}
void Context::Destroy() {
  if (m_device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(m_device);
  }
  if (m_debugMessenger != VK_NULL_HANDLE) {
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    m_debugMessenger = VK_NULL_HANDLE;
  }
  if (m_device != VK_NULL_HANDLE) {
    vkDestroyDevice(m_device, nullptr);
  }
  if (m_instance != VK_NULL_HANDLE) {
    vkDestroyInstance(m_instance, nullptr);
  }
}
}  // namespace ezg::vk