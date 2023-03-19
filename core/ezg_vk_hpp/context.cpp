#include "context.hpp"
#include <iostream>
#include "logging.hpp"

namespace ezg::vulkan {
#ifdef EZG_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              vk::DebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  switch (messageSeverity) {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
      spdlog::error("{}", pCallbackData->pMessage);
      break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
      spdlog::warn("{}", pCallbackData->pMessage);
      break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
      spdlog::info("{}", pCallbackData->pMessage);
      break;
    default:
      return VK_FALSE;
  }
  return VK_FALSE;
}
#endif

bool Context::CreateInstance(const char** exts, uint32_t extCount) {
  m_supported_exts   = vk::enumerateInstanceExtensionProperties();
  m_supported_layers = vk::enumerateInstanceLayerProperties();

  //  uint32_t glfwExtensionCount = 0;
  //  const char** glfwExtensions;
  //  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  bool has_surface_extension = false;
  for (auto i = 0u; i < extCount; i++) {
    if (!hasExtension(exts[i])) {
      return false;
    }
    if (strcmp(exts[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0) {
      has_surface_extension = true;
    }
    m_enabled_ext_names.push_back(exts[i]);
  }
  if (has_surface_extension && hasExtension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)) {
    m_enabled_ext_names.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    m_exts.supports_surface_capabilities2 = true;
  }
  if (has_surface_extension && hasExtension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)) {
    m_enabled_ext_names.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
    m_exts.supports_swapchain_colorspace = true;
  }
#ifdef EZG_DEBUG
  vk::ValidationFeaturesEXT validation_features{};
  if (hasLayer(ValidationLayerName.data())) {
    m_enabled_layer_names.push_back(ValidationLayerName.data());
    // Get the extensions supported by the validation layer
    auto layer_exts = vk::enumerateInstanceExtensionProperties(ValidationLayerName);
    if (std::find_if(layer_exts.begin(), layer_exts.end(), [](const vk::ExtensionProperties& e) {
          return strcmp(e.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
        }) != layer_exts.end()) {
      m_enabled_ext_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      m_exts.supports_debug_utils = true;
    }
    if (std::find_if(layer_exts.begin(), layer_exts.end(), [](const vk::ExtensionProperties& e) {
          return strcmp(e.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0;
        }) != end(layer_exts)) {
      m_enabled_ext_names.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
      static const vk::ValidationFeatureEnableEXT validation_sync_features[1] = {
          vk::ValidationFeatureEnableEXT::eSynchronizationValidation};
      spdlog::info("Enabling VK_EXT_validation_features for synchronization validation.");
      validation_features.enabledValidationFeatureCount = 1;
      validation_features.pEnabledValidationFeatures    = validation_sync_features;
    }
  }
#endif
  auto create_info =
      vk::InstanceCreateInfo(vk::InstanceCreateFlags(),
                             &defaultAppInfo(),  // application info
                             m_enabled_layer_names.size(), m_enabled_layer_names.data(),  // layer
                             m_enabled_ext_names.size(), m_enabled_ext_names.data(),  // extension
                             &validation_features);

  try {
    m_instance  = vk::createInstanceUnique(create_info);
    auto vk_dll = vk::DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);
#ifdef EZG_DEBUG
    if (m_exts.supports_debug_utils) {
      auto debug_ci = debugUtilMessengerCI();
      m_instance->createDebugUtilsMessengerEXTUnique(debug_ci, nullptr, vk_dll);
    }
#endif
  } catch (vk::SystemError& err) {
    spdlog::critical("Failed to create instance: {}!", err.what());
    throw std::runtime_error("Failed to create instance!");
  }

  displayInfo();
  return true;
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
bool Context::hasExtension(const char* name) {
  return std::find_if(m_supported_exts.begin(), m_supported_exts.end(),
                      [&](const vk::ExtensionProperties& ext) {
                        return strcmp(ext.extensionName.data(), name);
                      }) != m_supported_exts.end();
}

bool Context::hasLayer(const char* name) {
  return std::find_if(m_supported_layers.begin(), m_supported_layers.end(),
                      [&](const vk::LayerProperties& layer) {
                        return strcmp(layer.layerName.data(), name);
                      }) != m_supported_layers.end();
}

void Context::displayInfo() {
  spdlog::info("Available Instance extensions:");
  spdlog::set_pattern("%v");
  for (const auto& ext : m_supported_exts) {
    spdlog::info("\t{}", ext.extensionName);
  }
  logger::SetDefaultPattern();
  spdlog::info("Available Instance layers:");
  spdlog::set_pattern("%v");
  for (const auto& layer : m_supported_layers) {
    spdlog::info("\t{}", layer.layerName);
  }
  logger::SetDefaultPattern();

  spdlog::info("Enabled Instance extensions:");
  spdlog::set_pattern("%v");
  for (const auto& name : m_enabled_ext_names) {
    spdlog::info("\t{}", name);
  }
  logger::SetDefaultPattern();
  spdlog::info("Enabled Instance layers:");
  spdlog::set_pattern("%v");
  for (const auto& name : m_enabled_layer_names) {
    spdlog::info("\t{}", name);
  }
  logger::SetDefaultPattern();
}

const vk::ApplicationInfo& Context::defaultAppInfo() const {
  static const vk::ApplicationInfo appInfo{"ezg vulkan", VK_MAKE_VERSION(1, 0, 0), "No Engine",
                                           VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_1};
  return appInfo;
}
}  // namespace ezg::vulkan