#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include <vector>
#include "vk_base.hpp"
namespace ezg::vulkan {
struct DeviceExts {
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

class Context {
public:
  bool CreateInstance(const char** exts, uint32_t extCount);
  auto& GetInstance() const { return m_instance; }
private:
  bool hasExtension(const char* name);
  bool hasLayer(const char* name);
  vk::DebugUtilsMessengerCreateInfoEXT debugUtilMessengerCI();
  void displayInfo();
  const vk::ApplicationInfo& defaultAppInfo() const;

  vk::UniqueInstance m_instance;
  std::vector<vk::ExtensionProperties> m_supported_exts;
  std::vector<vk::LayerProperties> m_supported_layers;
  std::vector<const char*> m_enabled_ext_names;
  std::vector<const char*> m_enabled_layer_names;
  DeviceExts m_exts;
  const std::string ValidationLayerName = "VK_LAYER_KHRONOS_validation";
};
}  // namespace ezg::vulkan
#endif  //CONTEXT_HPP
