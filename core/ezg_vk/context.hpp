#ifndef EASYGRAPHICS_CONTEXT_HPP
#define EASYGRAPHICS_CONTEXT_HPP
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <volk.h>
#include <memory>
#include <vector>
#if defined(_WIN32)
#include <fcntl.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include "common.hpp"
#endif  // _WIN32

namespace ezg::vk {

struct DeviceFeatures {
  bool supports_debug_utils                      = false;
  bool supports_mirror_clamp_to_edge             = false;
  bool supports_google_display_timing            = false;
  bool supports_nv_device_diagnostic_checkpoints = false;
  bool supports_external_memory_host             = false;
  bool supports_surface_capabilities2            = false;
  bool supports_full_screen_exclusive            = false;
  bool supports_descriptor_indexing              = false;
  bool supports_conservative_rasterization       = false;
  bool supports_draw_indirect_count              = false;
  bool supports_driver_properties                = false;
  bool supports_calibrated_timestamps            = false;
  bool supports_memory_budget                    = false;
  bool supports_astc_decode_mode                 = false;
  bool supports_sync2                            = false;
  bool supports_video_queue                      = false;
  bool supports_video_decode_queue               = false;
  bool supports_video_decode_h264                = false;
  bool supports_pipeline_creation_cache_control  = false;
  bool supports_format_feature_flags2            = false;
  bool supports_external                         = false;
  bool supports_image_format_list                = false;
  bool supports_shader_float_control             = false;
  bool supports_tooling_info                     = false;
  bool supports_hdr_metadata                     = false;
  bool supports_swapchain_colorspace             = false;

  // Vulkan 1.1 core
  VkPhysicalDeviceFeatures enabled_features                                        = {};
  VkPhysicalDeviceMultiviewFeatures multiview_features                             = {};
  VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features     = {};
  VkPhysicalDeviceSamplerYcbcrConversionFeatures sampler_ycbcr_conversion_features = {};
  VkPhysicalDeviceMultiviewProperties multiview_properties                         = {};
  VkPhysicalDeviceSubgroupProperties subgroup_properties                           = {};

  // KHR
  VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timeline_semaphore_features = {};
  VkPhysicalDevicePerformanceQueryFeaturesKHR performance_query_features   = {};
  VkPhysicalDeviceDriverPropertiesKHR driver_properties                    = {};
  VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features               = {};
  VkPhysicalDevicePresentIdFeaturesKHR present_id_features                 = {};
  VkPhysicalDevicePresentWaitFeaturesKHR present_wait_features             = {};
  VkPhysicalDevice8BitStorageFeaturesKHR storage_8bit_features             = {};
  VkPhysicalDevice16BitStorageFeaturesKHR storage_16bit_features           = {};
  VkPhysicalDeviceFloat16Int8FeaturesKHR float16_int8_features             = {};
  VkPhysicalDeviceFloatControlsPropertiesKHR float_control_properties      = {};
  VkPhysicalDeviceIDProperties id_properties                               = {};

  // EXT
  VkPhysicalDeviceExternalMemoryHostPropertiesEXT host_memory_properties            = {};
  VkPhysicalDeviceSubgroupSizeControlFeaturesEXT subgroup_size_control_features     = {};
  VkPhysicalDeviceSubgroupSizeControlPropertiesEXT subgroup_size_control_properties = {};
  VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features               = {};
  VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT demote_to_helper_invocation_features =
      {};
  VkPhysicalDeviceScalarBlockLayoutFeaturesEXT scalar_block_features                           = {};
  VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR ubo_std430_features                   = {};
  VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing_features                   = {};
  VkPhysicalDeviceDescriptorIndexingPropertiesEXT descriptor_indexing_properties               = {};
  VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_properties = {};
  VkPhysicalDeviceMemoryPriorityFeaturesEXT memory_priority_features                           = {};
  VkPhysicalDeviceASTCDecodeFeaturesEXT astc_decode_features                                   = {};
  VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT astc_hdr_features                       = {};
  VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT pipeline_creation_cache_control_features =
      {};

  // Vendor
  VkPhysicalDeviceComputeShaderDerivativesFeaturesNV compute_shader_derivative_features = {};
};

class VulkanLib {
public:
  static VulkanLib& Get();
  ~VulkanLib();

private:
  VulkanLib();

  template <typename T>
  void LoadFunc(T& func_dest, const char* func_name);

#if defined(__linux__) || defined(__APPLE__)
  void* library;
#elif defined(_WIN32)
  HMODULE m_library;
#endif

  PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr{VK_NULL_HANDLE};

  friend class Context;
};

template <typename T>
void VulkanLib::LoadFunc(T& func_dest, const char* func_name) {
#if defined(__linux__) || defined(__APPLE__)
  func_dest = reinterpret_cast<T>(dlsym(m_library, func_name));
#elif defined(_WIN32)
  func_dest = reinterpret_cast<T>(GetProcAddress(m_library, func_name));
#endif
}

class Instance {
public:
  [[nodiscard]] VkInstance Handle() const { return m_instance; };

private:
  VkInstance m_instance{VK_NULL_HANDLE};
};

class Device {
public:
private:
  VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
  VkDevice m_device{VK_NULL_HANDLE};
};
struct ContextCreateInfo {
  uint32_t enabledLayerCount{0};
  const char** ppEnabledInstLayers{nullptr};

  uint32_t enabledInstExtCount{0};
  const char** ppEnabledInstExts;

  uint32_t enabledDeviceExtCount{0};
  const char** ppEnabledDeviceExts{nullptr};

  SDL_Window* window;

  const std::vector<VkBaseOutStructure*> pNexts;
};

struct CustomQueueInfo {
  CustomQueueInfo() {
    for (auto& index : familyIndices)
      index = VK_QUEUE_FAMILY_IGNORED;
  }
  VkQueue queues[QUEUE_INDEX_COUNT]         = {};
  uint32_t familyIndices[QUEUE_INDEX_COUNT] = {};
};

bool CreateContext(const ContextCreateInfo& contextCreateInfo, Context* ctx);

class Context {
public:
  static bool InitLoader(PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr = nullptr);
  static PFN_vkGetInstanceProcAddr InstanceFuncLoader();
  static PFN_vkGetDeviceProcAddr DeviceFuncLoader();
  [[nodiscard]] VkInstance GetVkInstance() const { return m_instance; }
  explicit Context();
  ~Context();

  Context& DisableValidation() {
    m_enableValidation = false;
    return *this;
  }

  //  bool InitInstance()

  bool CreateInstance();
  bool CreateDevice();

private:
  void SelectGPU();
  bool PopulateQueueInfo();
  void Destroy();
  uint32_t version = VK_API_VERSION_1_1;
  DeviceFeatures ext;
  VkApplicationInfo m_appInfo{};
  std::vector<VkLayerProperties> m_instLayers;
  std::vector<VkExtensionProperties> m_instanceExts;
  std::vector<VkExtensionProperties> m_deviceExts;

  std::vector<const char*> m_requiredInstLayers;
  std::vector<const char*> m_requiredInstExts;
  std::vector<const char*> m_requiredDeviceExts;
  // enable validation layer by default
  bool m_enableValidation = true;
  VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};

  VolkDeviceTable deviceTable{};
  VkInstance m_instance{VK_NULL_HANDLE};
  VkPhysicalDevice m_gpu{VK_NULL_HANDLE};
  VkPhysicalDeviceProperties m_gpuProps{};
  VkPhysicalDeviceMemoryProperties m_gpuMemProps{};
  VkDevice m_device{VK_NULL_HANDLE};
  std::vector<VkBaseOutStructure*> m_pNextChain;

  std::vector<VkDeviceQueueCreateInfo> m_qCreateInfos;
  CustomQueueInfo m_customQInfo;
  uint32_t m_qIndices[QUEUE_INDEX_COUNT] = {};
  std::vector<std::vector<float>> m_qPriorities;

  VkSurfaceKHR m_surface{VK_NULL_HANDLE};
  SDL_Window* m_window{nullptr};

  friend bool CreateContext(const ContextCreateInfo& contextCreateInfo, Context* ctx);
};

}  // namespace ezg::vk
#endif  //EASYGRAPHICS_CONTEXT_HPP
