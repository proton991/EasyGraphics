#include "context.hpp"
#include <mutex>
#include "common.hpp"
#include "spdlog/spdlog.h"

namespace spd = spdlog;
namespace ezg::vk {

static std::mutex mutex;
static PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr{VK_NULL_HANDLE};
//static PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr{VK_NULL_HANDLE};
static bool loaderInit = false;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_util_messenger_cb(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  auto* context = static_cast<Context*>(pUserData);

  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        spd::error("[Vulkan]: {}", pCallbackData->pMessage);
      } else
        spd::error("[Vulkan] Other Error: {}", pCallbackData->pMessage);
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
      spd::error("  Object {}: {}", i, name ? name : "N/A");
    }
  }

  return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT DebugUtilMessengerCI() {
  VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
  messengerCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messengerCreateInfo.pNext           = nullptr;
  messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
  messengerCreateInfo.pfnUserCallback = debug_util_messenger_cb;
  messengerCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  return messengerCreateInfo;
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

Context::Context() {
  //query instance layers
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  m_instLayers.resize(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, m_instLayers.data());
  spd::info("Layer Count: {}", layerCount);
  for (auto& layer : m_instLayers) {
    spd::info("Found layer: {}", layer.layerName);
  }
  // query instance extensions
  uint32_t instanceExtCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtCount, nullptr);
  m_instanceExts.resize(instanceExtCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtCount, m_instanceExts.data());
  spd::info("Instance Extension Count: {}", instanceExtCount);
  for (auto& instanceExt : m_instanceExts) {
    spd::info("Found instance extension: {}", instanceExt.extensionName);
  }
  m_appInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  m_appInfo.apiVersion       = VK_API_VERSION_1_1;
  m_appInfo.engineVersion    = VK_API_VERSION_1_1;
  m_appInfo.pNext            = nullptr;
  m_appInfo.pApplicationName = "Easy Graphics With Vulkan";
  m_appInfo.pEngineName      = "No Engine";
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
    auto layer_itr = find_if(m_instLayers.begin(), m_instLayers.end(),
                             [name](const VkLayerProperties& e) -> bool {
                               return strcmp(e.layerName, name) == 0;
                             });
    return layer_itr != m_instLayers.end();
  };
  VkInstanceCreateInfo instanceInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  instanceInfo.pApplicationInfo     = &m_appInfo;

  VkValidationFeaturesEXT validationFeatures = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT};
  std::vector<const char*> instanceExts;
  std::vector<const char*> instanceLayers;
  instanceExts.insert(instanceExts.end(), m_requiredInstExts.begin(), m_requiredInstExts.end());
  instanceLayers.insert(instanceLayers.end(), m_requiredInstLayers.begin(),
                        m_requiredInstLayers.end());

  if (has_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
    instanceExts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    ext.supports_debug_utils = true;
  }

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

  std::vector<VkBaseOutStructure*> pNextChain;
  if (m_enableValidation && ext.supports_debug_utils) {
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = DebugUtilMessengerCI();
    messengerCreateInfo.pUserData                          = nullptr;
    pNextChain.push_back(reinterpret_cast<VkBaseOutStructure*>(&messengerCreateInfo));
  }
  SetPNextChain(instanceInfo, pNextChain);
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
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = DebugUtilMessengerCI();
    debugInfo.pUserData                          = this;

    // For some reason, this segfaults Android, sigh ... We get relevant output in logcat anyways.
    if (vkCreateDebugUtilsMessengerEXT)
      vkCreateDebugUtilsMessengerEXT(m_instance, &debugInfo, nullptr, &m_debugMessenger);
  }
  return true;
}

void Context::SelectGPU() {
  uint32_t gpuCount = 0;
  Check(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr), "enumerate gpu count");
  if (gpuCount == 0) {
    spd::critical("No Vulkan GPU found!");
    return;
  }
  std::vector<VkPhysicalDevice> gpus(gpuCount);
  Check(vkEnumeratePhysicalDevices(m_instance, &gpuCount, gpus.data()), "enumerate gpu");
  for (auto& gpu : gpus) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(gpu, &props);
    spd::info("Found Vulkan GPU: {}", props.deviceName);
    spd::info("    API: {}.{}.{}", VK_VERSION_MAJOR(props.apiVersion),
              VK_VERSION_MINOR(props.apiVersion), VK_VERSION_PATCH(props.apiVersion));
    spd::info("    Driver: {}.{}.{}\n", VK_VERSION_MAJOR(props.driverVersion),
              VK_VERSION_MINOR(props.driverVersion), VK_VERSION_PATCH(props.driverVersion));
    // select first discrete gpu that has proper api version
    if (m_gpu == VK_NULL_HANDLE && props.apiVersion >= VK_API_VERSION_1_1 &&
        props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      m_gpu = gpu;
    }
  }
  if (m_gpu != VK_NULL_HANDLE) {
    vkGetPhysicalDeviceProperties(m_gpu, &m_gpuProps);
    vkGetPhysicalDeviceMemoryProperties(m_gpu, &m_gpuMemProps);
  } else {
    spd::critical("No Vulkan GPU found!");
  }
  // query device extensions
  uint32_t extCount = 0;
  vkEnumerateDeviceExtensionProperties(m_gpu, nullptr, &extCount, nullptr);
  m_deviceExts.resize(extCount);
  vkEnumerateDeviceExtensionProperties(m_gpu, nullptr, &extCount, m_deviceExts.data());
  for (auto& deviceExt : m_deviceExts) {
    spd::info("Found device extension: {}", deviceExt.extensionName);
  }
}

bool Context::PopulateQueueInfo() {
  // query queue family and queue family indices
  uint32_t qFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &qFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> qProps(qFamilyCount);
  std::vector<uint32_t> qOffsets(qFamilyCount);
  //  std::vector<std::vector<float>> qPriorities(qFamilyCount);
  m_qPriorities.resize(qFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &qFamilyCount, qProps.data());
  m_customQInfo = {};

  const auto find_vacant_queue = [&](uint32_t& family, uint32_t& index, VkQueueFlags required,
                                     VkQueueFlags ignore_flags, float priority) -> bool {
    for (unsigned familyIndex = 0; familyIndex < qFamilyCount; familyIndex++) {
      if ((qProps[familyIndex].queueFlags & ignore_flags) != 0)
        continue;

      if (qProps[familyIndex].queueCount &&
          (qProps[familyIndex].queueFlags & required) == required) {
        family = familyIndex;
        qProps[familyIndex].queueCount--;
        index = qOffsets[familyIndex]++;
        m_qPriorities[familyIndex].push_back(priority);
        return true;
      }
    }
    return false;
  };
  if (!find_vacant_queue(m_customQInfo.familyIndices[QUEUE_INDEX_GRAPHICS],
                         m_customQInfo.qIndices[QUEUE_INDEX_GRAPHICS],
                         VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 0.5f)) {
    spd::error("Could not find suitable graphics queue.");
    return false;
  }
  // Prefer another graphics queue since we can do async graphics that way.
  // The compute queue is to be treated as high priority since we also do async graphics on it.
  if (!find_vacant_queue(m_customQInfo.familyIndices[QUEUE_INDEX_COMPUTE],
                         m_customQInfo.qIndices[QUEUE_INDEX_COMPUTE],
                         VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 1.0f) &&
      !find_vacant_queue(m_customQInfo.familyIndices[QUEUE_INDEX_COMPUTE],
                         m_customQInfo.qIndices[QUEUE_INDEX_COMPUTE], VK_QUEUE_COMPUTE_BIT, 0, 1.0f)) {
    // Fallback to the graphics queue if we must.
    m_customQInfo.familyIndices[QUEUE_INDEX_COMPUTE] =
        m_customQInfo.familyIndices[QUEUE_INDEX_GRAPHICS];
    m_customQInfo.qIndices[QUEUE_INDEX_COMPUTE] = m_customQInfo.qIndices[QUEUE_INDEX_GRAPHICS];
  }

  // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
  // If not, fallback to a dedicated compute queue.
  // Finally, fallback to same queue as compute.
  if (!find_vacant_queue(m_customQInfo.familyIndices[QUEUE_INDEX_TRANSFER],
                         m_customQInfo.qIndices[QUEUE_INDEX_TRANSFER], VK_QUEUE_TRANSFER_BIT,
                         VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0.5f) &&
      !find_vacant_queue(m_customQInfo.familyIndices[QUEUE_INDEX_TRANSFER],
                         m_customQInfo.qIndices[QUEUE_INDEX_TRANSFER], VK_QUEUE_COMPUTE_BIT,
                         VK_QUEUE_GRAPHICS_BIT, 0.5f)) {
    m_customQInfo.familyIndices[QUEUE_INDEX_TRANSFER] =
        m_customQInfo.familyIndices[QUEUE_INDEX_COMPUTE];
    m_customQInfo.qIndices[QUEUE_INDEX_TRANSFER] = m_customQInfo.qIndices[QUEUE_INDEX_COMPUTE];
  }

  std::vector<float> defaultPriorities = std::vector<float>{1.0f};
  for (uint32_t familyIndex = 0; familyIndex < qFamilyCount; familyIndex++) {
    if (qOffsets[familyIndex] == 0)
      continue;

    VkDeviceQueueCreateInfo info = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    info.queueFamilyIndex        = familyIndex;
    info.queueCount              = qOffsets[familyIndex];
    info.pQueuePriorities        = m_qPriorities[familyIndex].data();
    m_qCreateInfos.push_back(info);
  }
  return true;
}

bool Context::CreateDevice() {
  SelectGPU();
  const auto has_extension = [&](const char* name) -> bool {
    auto itr = find_if(begin(m_deviceExts), end(m_deviceExts),
                       [name](const VkExtensionProperties& e) -> bool {
                         return strcmp(e.extensionName, name) == 0;
                       });
    return itr != end(m_deviceExts);
  };
  std::vector<const char*> enabledExts;
  for (auto& extension : m_requiredDeviceExts) {
    if (!has_extension(extension)) {
      spd::critical("Device extension {} not supported.", extension);
      return false;
    }
    enabledExts.push_back(extension);
  }
  PopulateQueueInfo();
  VkDeviceCreateInfo deviceInfo      = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  deviceInfo.pQueueCreateInfos       = m_qCreateInfos.data();
  deviceInfo.queueCreateInfoCount    = static_cast<uint32_t>(m_qCreateInfos.size());
  deviceInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExts.size());
  deviceInfo.ppEnabledExtensionNames = enabledExts.data();
  SetPNextChain(deviceInfo, m_pNextChain);
  for (auto& extension : enabledExts) {
    spd::info("Enabling device extension {}", extension);
  }

  Check(vkCreateDevice(m_gpu, &deviceInfo, nullptr, &m_device), "create device");
  volkLoadDevice(m_device);
  for (int i = 0; i < QUEUE_INDEX_COUNT; i++) {
    if (m_customQInfo.familyIndices[i] != VK_QUEUE_FAMILY_IGNORED) {
      vkGetDeviceQueue(m_device, m_customQInfo.familyIndices[i], m_customQInfo.qIndices[i],
                       &m_customQInfo.queues[i]);
    } else {
      m_customQInfo.queues[i] = VK_NULL_HANDLE;
    }
  }

  return true;
}

bool CreateContext(const ContextCreateInfo& ctxInfo, Context* ctx) {
  for (int i = 0; i < ctxInfo.enabledLayerCount; ++i) {
    ctx->m_requiredInstLayers.push_back(ctxInfo.ppEnabledInstLayers[i]);
  }
  for (int i = 0; i < ctxInfo.enabledInstExtCount; ++i) {
    ctx->m_requiredInstExts.push_back(ctxInfo.ppEnabledInstExts[i]);
  }
  for (int i = 0; i < ctxInfo.enabledDeviceExtCount; ++i) {
    ctx->m_requiredDeviceExts.push_back(ctxInfo.ppEnabledDeviceExts[i]);
  }
  for (auto& pNext : ctxInfo.pNexts) {
    ctx->m_pNextChain.push_back(pNext);
  }

  if (!ctx->CreateInstance()) {
    spd::critical("Failed to create vulkan instance");
    return false;
  }

  if (!ctx->CreateDevice()) {
    spd::critical("Failed to create vulkan device");
    return false;
  }

  DebugUtil::Get().SetObjectName(ctx->m_device, "device");

  return true;
}
Context::~Context() {
  Destroy();
}
void Context::Destroy() {
  if (m_device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(m_device);
  }
//  if (m_surface != VK_NULL_HANDLE) {
//    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
//  }
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