#include "vk_tools.hpp"
#include "vk_device.hpp"
#include "vk_functions.hpp"
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdexcept>
#include <string>

#pragma once

namespace vkh {

void VkCheck(const VkResult result, const char* const operation) {
  if (result != VK_SUCCESS) {
    std::cerr << std::string("failed to ") + operation + " (" + ToString(result) + ")" << std::endl;
    abort();
  }
}

const char* ToString(const VkResult result) {
  switch (result) {
#define STR(r) \
  case VK_##r: \
    return #r
    STR(SUCCESS);
    STR(NOT_READY);
    STR(TIMEOUT);
    STR(EVENT_SET);
    STR(EVENT_RESET);
    STR(INCOMPLETE);
    STR(ERROR_OUT_OF_HOST_MEMORY);
    STR(ERROR_OUT_OF_DEVICE_MEMORY);
    STR(ERROR_INITIALIZATION_FAILED);
    STR(ERROR_DEVICE_LOST);
    STR(ERROR_MEMORY_MAP_FAILED);
    STR(ERROR_LAYER_NOT_PRESENT);
    STR(ERROR_EXTENSION_NOT_PRESENT);
    STR(ERROR_FEATURE_NOT_PRESENT);
    STR(ERROR_INCOMPATIBLE_DRIVER);
    STR(ERROR_TOO_MANY_OBJECTS);
    STR(ERROR_FORMAT_NOT_SUPPORTED);
    STR(ERROR_FRAGMENTED_POOL);
    STR(ERROR_UNKNOWN);
    STR(ERROR_OUT_OF_POOL_MEMORY);
    STR(ERROR_INVALID_EXTERNAL_HANDLE);
    STR(ERROR_FRAGMENTATION);
    STR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
    STR(ERROR_SURFACE_LOST_KHR);
    STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
    STR(SUBOPTIMAL_KHR);
    STR(ERROR_OUT_OF_DATE_KHR);
    STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
    STR(ERROR_VALIDATION_FAILED_EXT);
    STR(ERROR_INVALID_SHADER_NV);
    STR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
    STR(ERROR_NOT_PERMITTED_EXT);
    STR(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
    STR(THREAD_IDLE_KHR);
    STR(THREAD_DONE_KHR);
    STR(OPERATION_DEFERRED_KHR);
    STR(OPERATION_NOT_DEFERRED_KHR);
    STR(PIPELINE_COMPILE_REQUIRED_EXT);
#undef STR
    default:
      return "UNKNOWN_ERROR";
  }
}
bool CheckExtensionSupported(const std::vector<VkExtensionProperties>& availableExtensions,
                             const char* extensionName) {
  for (const auto& ext : availableExtensions) {
    if (strcmp(ext.extensionName, extensionName) == 0) {
      return true;
    }
  }
  return false;
}
bool CheckExtensionsSupported(const std::vector<VkExtensionProperties>& availableExtensions,
                              const std::vector<const char*>& extensionNames) {
  for (const auto& extensionName : extensionNames) {
    if (!CheckExtensionSupported(availableExtensions, extensionName)) {
      return false;
    }
  }
  return true;
}

bool CheckLayerSupported(std::vector<VkLayerProperties> const& availableLayers,
                         const char* layerName) {
  if (!layerName)
    return false;
  for (const auto& layerProp : availableLayers) {
    if (strcmp(layerName, layerProp.layerName) == 0) {
      return true;
    }
  }
  return false;
}

bool CheckLayersSupported(std::vector<VkLayerProperties> const& availableLayers,
                          std::vector<const char*> const& layerNames) {
  for (const auto& layerName : layerNames) {
    if (!CheckLayerSupported(availableLayers, layerName)) {
      return false;
    }
  }
  return true;
}

int Console::SetColorBySeverity(const Severity severity) noexcept {
#ifdef WIN32
  const HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);

  CONSOLE_SCREEN_BUFFER_INFO info = {};
  GetConsoleScreenBufferInfo(hConsole, &info);

  switch (severity) {
    case Severity::Verbos:
      SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
      break;
    case Severity::Info:
      SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
      break;
    case Severity::Warning:
      SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
      break;
    case Severity::Error:
      SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
      break;
    case Severity::Fatal:
      SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
      break;
    default:;
  }

  return info.wAttributes;
#else
  return 0;
#endif
}

void Console::SetColorByAttributes(const int attributes) noexcept {
#ifdef WIN32
  const HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
  SetConsoleTextAttribute(hConsole, attributes);
#endif
}

}  // namespace vkh