#include "context.hpp"
#include "logging.hpp"

namespace ezg::vulkan {
#ifdef EZG_DEBUG
static VKAPI_ATTR vk::Bool32 VKAPI_CALL
debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              vk::DebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  switch (messageSeverity) {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
      logger::error("{}", pCallbackData->pMessage);
      break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
      logger::warn("{}", pCallbackData->pMessage);
      break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
      logger::info("{}", pCallbackData->pMessage);
      break;
    default:
      return VK_FALSE;
  }
  return VK_FALSE;
}
#endif

bool Context::InitInstanceAndDevice(const ContextCreateInfo& context_ci) {
  if (!createInstance(context_ci.instance_exts)) {
    logger::critical("Failed to create instance!");
    return false;
  }
  if (!createDevice(context_ci.surface, context_ci.device_exts)) {
    logger::critical("Failed to create device!");
    return false;
  }
  return true;
}

bool Context::createInstance(const std::vector<const char*>& instance_exts) {
  auto supported_exts   = vk::enumerateInstanceExtensionProperties();
  auto supported_layers = vk::enumerateInstanceLayerProperties();

  //  uint32_t glfwExtensionCount = 0;
  //  const char** glfwExtensions;
  //  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char*> enabled_ext_names;
  std::vector<const char*> enabled_layer_names;
  bool has_surface_extension = false;
  for (const auto& ext : instance_exts) {
    if (!hasExtension(ext, supported_exts)) {
      return false;
    }
    if (strcmp(ext, VK_KHR_SURFACE_EXTENSION_NAME) == 0) {
      has_surface_extension = true;
    }
    enabled_ext_names.push_back(ext);
  }
  if (has_surface_extension &&
      hasExtension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, supported_exts)) {
    enabled_ext_names.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    m_features.supports_surface_capabilities2 = true;
  }
  if (has_surface_extension &&
      hasExtension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, supported_exts)) {
    enabled_ext_names.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
    m_features.supports_swapchain_colorspace = true;
  }
#ifdef EZG_DEBUG
  vk::ValidationFeaturesEXT validation_features{};
  if (hasLayer(ValidationLayerName.data(), supported_layers)) {
    enabled_layer_names.push_back(ValidationLayerName.data());
    // Get the extensions supported by the validation layer
    auto layer_exts = vk::enumerateInstanceExtensionProperties(ValidationLayerName);
    if (std::find_if(layer_exts.begin(), layer_exts.end(), [](const vk::ExtensionProperties& e) {
          return strcmp(e.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
        }) != layer_exts.end()) {
      enabled_ext_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      m_features.supports_debug_utils = true;
    }
    if (std::find_if(layer_exts.begin(), layer_exts.end(), [](const vk::ExtensionProperties& e) {
          return strcmp(e.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0;
        }) != end(layer_exts)) {
      enabled_ext_names.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
      static const vk::ValidationFeatureEnableEXT validation_sync_features[1] = {
          vk::ValidationFeatureEnableEXT::eSynchronizationValidation};
      logger::info("Enabling VK_EXT_validation_features for synchronization validation.");
      validation_features.enabledValidationFeatureCount = 1;
      validation_features.pEnabledValidationFeatures    = validation_sync_features;
    }
  }
#endif
  auto create_info =
      vk::InstanceCreateInfo(vk::InstanceCreateFlags(),
                             &defaultAppInfo(),  // application info
                             enabled_layer_names.size(), enabled_layer_names.data(),  // layer
                             enabled_ext_names.size(), enabled_ext_names.data(),      // extension
                             &validation_features);

  try {
    m_instance  = vk::createInstanceUnique(create_info);
    auto vk_dll = vk::DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);
#ifdef EZG_DEBUG
    if (m_features.supports_debug_utils) {
      auto debug_ci = debugUtilMessengerCI();
      m_instance->createDebugUtilsMessengerEXTUnique(debug_ci, nullptr, vk_dll);
    }
  } catch (vk::SystemError& err) {
    logger::critical("Failed to create instance: {}!", err.what());
    throw std::runtime_error("Failed to create instance!");
  }
#endif

#ifdef EZG_DEBUG
  logger::info("Enabled Instance extensions:");
  logger::LogList(enabled_ext_names);

  logger::info("Enabled Instance layers:");
  logger::LogList(enabled_layer_names);
#endif
  return true;
}

bool Context::createDevice(vk::SurfaceKHR surface, const std::vector<const char*>& device_exts) {
  m_surface = surface;
  selectGPU();
  auto supported_exts = m_gpu.enumerateDeviceExtensionProperties(nullptr);
  std::vector<const char*> enabled_ext_names;
  for (const auto& ext : device_exts) {
    if (!hasExtension(ext, supported_exts)) {
      return false;
    }
    enabled_ext_names.push_back(ext);
  }
  std::vector<vk::DeviceQueueCreateInfo> queue_ci;
  if (!populateQueueCI(queue_ci)) {
    return false;
  }
  auto device_ci                    = vk::DeviceCreateInfo();
  device_ci.flags                   = vk::DeviceCreateFlags();
  device_ci.pQueueCreateInfos       = queue_ci.data();
  device_ci.queueCreateInfoCount    = queue_ci.size();
  device_ci.ppEnabledExtensionNames = enabled_ext_names.data();
  device_ci.enabledExtensionCount   = enabled_ext_names.size();
  try {
    m_device = m_gpu.createDeviceUnique(device_ci);
  } catch (vk::SystemError& e) {
    logger::critical("Failed to create logical device!");
    throw std::runtime_error("Failed to create logical device!");
  }
#ifdef EZG_DEBUG
  logger::info("Enabled Device Extensions:");
  logger::LogList(enabled_ext_names);
#endif

  return true;
}

bool Context::findProperQueue(uint32_t& family, uint32_t& index, vk::QueueFlags required,
                              vk::QueueFlags ignored, float priority) {
  for (unsigned family_index = 0; family_index < m_queue_family_count; family_index++) {
    if (m_queue_props[family_index].queueFlags & ignored)
      continue;

    // A graphics queue candidate must support present for us to select it.
    if ((required & vk::QueueFlagBits::eGraphics) && m_surface) {
      VkBool32 supported = VK_FALSE;
      if (m_gpu.getSurfaceSupportKHR(family_index, m_surface, &supported) != vk::Result::eSuccess ||
          !supported)
        continue;
    }

    if (m_queue_props[family_index].queueCount &&
        (m_queue_props[family_index].queueFlags & required) == required) {
      family = family_index;
      m_queue_props[family_index].queueCount--;
      index = m_queue_offsets[family_index]++;
      m_queue_priorities[family_index].push_back(priority);
      return true;
    }
  }

  return false;
}

bool Context::populateQueueCI(std::vector<vk::DeviceQueueCreateInfo>& queue_cis) {

  m_queue_props        = m_gpu.getQueueFamilyProperties();
  m_queue_family_count = m_queue_props.size();
  m_queue_offsets.resize(m_queue_family_count);
  m_queue_priorities.resize(m_queue_family_count);
  if (!findProperQueue(
          m_q_info.family_indices[QUEUE_INDEX_GRAPHICS],               // family index
          m_q_info.queue_indices[QUEUE_INDEX_GRAPHICS],                // queue index
          vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute,  // required flags
          EMPTY_QUEUE_FLAG,                                            // ignore flags
          0.5f                                                         // priority
          )) {
    logger::error("Failed to find graphics queue!");
    return false;
  }
  // XXX: This assumes timestamp valid bits is the same for all queue types.
  m_q_info.timestamp_valid_bits =
      m_queue_props[m_q_info.family_indices[QUEUE_INDEX_GRAPHICS]].timestampValidBits;
  // Prefer another graphics queue since we can do async graphics that way.
  // The compute queue is to be treated as high priority since we also do async graphics on it.
  if (!findProperQueue(
          m_q_info.family_indices[QUEUE_INDEX_COMPUTE], m_q_info.queue_indices[QUEUE_INDEX_COMPUTE],
          vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute, EMPTY_QUEUE_FLAG, 1.0f) &&
      !findProperQueue(m_q_info.family_indices[QUEUE_INDEX_COMPUTE],
                       m_q_info.queue_indices[QUEUE_INDEX_COMPUTE], vk::QueueFlagBits::eCompute,
                       EMPTY_QUEUE_FLAG, 1.0f)) {
    // Fallback to the graphics queue if we must.
    m_q_info.family_indices[QUEUE_INDEX_COMPUTE] = m_q_info.family_indices[QUEUE_INDEX_GRAPHICS];
    m_q_info.queue_indices[QUEUE_INDEX_COMPUTE]  = m_q_info.queue_indices[QUEUE_INDEX_GRAPHICS];
  }

  // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
  // If not, fallback to a dedicated compute queue.
  // Finally, fallback to same queue as compute.
  if (!findProperQueue(m_q_info.family_indices[QUEUE_INDEX_TRANSFER],
                       m_q_info.queue_indices[QUEUE_INDEX_TRANSFER], vk::QueueFlagBits::eTransfer,
                       vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute, 0.5f) &&
      !findProperQueue(m_q_info.family_indices[QUEUE_INDEX_TRANSFER],
                       m_q_info.queue_indices[QUEUE_INDEX_TRANSFER], vk::QueueFlagBits::eCompute,
                       vk::QueueFlagBits::eGraphics, 0.5f)) {
    m_q_info.family_indices[QUEUE_INDEX_TRANSFER] = m_q_info.family_indices[QUEUE_INDEX_COMPUTE];
    m_q_info.queue_indices[QUEUE_INDEX_TRANSFER]  = m_q_info.queue_indices[QUEUE_INDEX_COMPUTE];
  }
  for (auto family_index = 0u; family_index < m_queue_family_count; family_index++) {
    if (m_queue_offsets[family_index] == 0) {
      continue;
    }
    auto queue_ci             = vk::DeviceQueueCreateInfo();
    queue_ci.flags            = vk::DeviceQueueCreateFlags();
    queue_ci.queueFamilyIndex = family_index;
    queue_ci.queueCount       = m_queue_offsets[family_index];
    queue_ci.pQueuePriorities = m_queue_priorities[family_index].data();
    queue_cis.push_back(queue_ci);
  }
  return true;
}

void Context::selectGPU() {
  auto gpus = m_instance->enumeratePhysicalDevices();
  if (gpus.empty()) {
    logger::critical("No Vulkan GPU found!");
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
  }
  for (auto& gpu : gpus) {
    auto props = gpu.getProperties();
    logger::info("Found Vulkan GPU: {}", props.deviceName);
    logger::SetListPattern();
    logger::info("API: {}.{}.{}", VK_VERSION_MAJOR(props.apiVersion),
                 VK_VERSION_MINOR(props.apiVersion), VK_VERSION_PATCH(props.apiVersion));
    logger::info("Driver: {}.{}.{}", VK_VERSION_MAJOR(props.driverVersion),
                 VK_VERSION_MINOR(props.driverVersion), VK_VERSION_PATCH(props.driverVersion));
    logger::SetDefaultPattern();
    // select first discrete gpu that has proper api version
    if (props.apiVersion >= VK_API_VERSION_1_1 &&
        props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
      m_gpu = gpu;
      break;
    }
  }
  m_gpu_props = m_gpu.getProperties();
}

vk::DebugUtilsMessengerCreateInfoEXT Context::debugUtilMessengerCI() {
  auto message_severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                          vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
  auto message_type = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
  auto debug_info = vk::DebugUtilsMessengerCreateInfoEXT();
  // fill in data
  debug_info.flags           = vk::DebugUtilsMessengerCreateFlagsEXT();
  debug_info.messageSeverity = message_severity;  // severity
  debug_info.messageType     = message_type;      // type
  debug_info.pfnUserCallback =
      reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(debugCallback);  // callback
  debug_info.pUserData = nullptr;

  return debug_info;
}

bool Context::hasExtension(const char* name, const std::vector<vk::ExtensionProperties>& exts) {
  return std::find_if(exts.begin(), exts.end(), [&](const vk::ExtensionProperties& ext) {
           return strcmp(ext.extensionName.data(), name);
         }) != exts.end();
}
bool Context::hasLayer(const char* name, const std::vector<vk::LayerProperties>& layers) {
  return std::find_if(layers.begin(), layers.end(), [&](const vk::LayerProperties& layer) {
           return strcmp(layer.layerName.data(), name);
         }) != layers.end();
}

const vk::ApplicationInfo& Context::defaultAppInfo() {
  static const vk::ApplicationInfo appInfo{"ezg vulkan", VK_MAKE_VERSION(1, 0, 0), "No Engine",
                                           VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_1};
  return appInfo;
}

QueueInfo::QueueInfo() {
  for (auto& index : family_indices)
    index = VK_QUEUE_FAMILY_IGNORED;
}
}  // namespace ezg::vulkan