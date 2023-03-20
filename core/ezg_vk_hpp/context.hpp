#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include <vector>
#include "common.hpp"
#include "vk_base.hpp"

namespace ezg::vulkan {
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
};

struct QueueInfo {
  QueueInfo();
  vk::Queue queues[QUEUE_INDEX_COUNT]        = {};
  uint32_t family_indices[QUEUE_INDEX_COUNT] = {};
  uint32_t queue_indices[QUEUE_INDEX_COUNT]  = {};
  uint32_t timestamp_valid_bits              = 0;
};

struct ContextCreateInfo {
  std::vector<const char*> instance_exts;
  std::vector<const char*> device_exts;
  vk::SurfaceKHR surface;
};
class Context {
public:
  bool InitInstanceAndDevice(const ContextCreateInfo& context_ci);

private:
  bool createInstance(const std::vector<const char*>& instance_exts);
  bool createDevice(vk::SurfaceKHR surface, const std::vector<const char*>& device_exts);
  void selectGPU();
  bool populateQueueCI(std::vector<vk::DeviceQueueCreateInfo>& queue_cis);
  bool findProperQueue(uint32_t& family, uint32_t& index, vk::QueueFlags required,
                       vk::QueueFlags ignored, float priority);
  bool hasExtension(const char* name, const std::vector<vk::ExtensionProperties>& exts);
  bool hasLayer(const char* name, const std::vector<vk::LayerProperties>& layers);

  vk::DebugUtilsMessengerCreateInfoEXT debugUtilMessengerCI();
  [[nodiscard]] static const vk::ApplicationInfo& defaultAppInfo();

  vk::UniqueInstance m_instance{nullptr};
  vk::PhysicalDevice m_gpu{nullptr};
  vk::UniqueDevice m_device{nullptr};
  vk::SurfaceKHR m_surface{nullptr};

  vk::PhysicalDeviceProperties m_gpu_props;

  uint32_t m_queue_family_count{0};
  std::vector<vk::QueueFamilyProperties> m_queue_props;
  std::vector<uint32_t> m_queue_offsets;
  std::vector<std::vector<float>> m_queue_priorities;
  QueueInfo m_q_info{};
  DeviceFeatures m_features;
  const std::string ValidationLayerName = "VK_LAYER_KHRONOS_validation";

  friend class Device;
};
}  // namespace ezg::vulkan
#endif  //CONTEXT_HPP
