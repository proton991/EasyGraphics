#ifndef RENDER_OPTION_HPP
#define RENDER_OPTION_HPP
namespace ezg::gl {
enum class LightType : decltype(0) { Point = 0, Directional = 1 };
struct RenderOptions {
  RenderOptions() = default;
  const char** model_list{nullptr};
  uint32_t num_models{0};
  int selected_model{0};
  bool rotate_model{false};
  bool rotate_camera{false};
  bool has_env_map{true};
  bool enable_env_map{true};
  bool show_bg{true};
  bool show_axis{false};
  bool show_aabb{false};
  bool show_floor{true};
  bool show_light_model{true};
  bool blur{false};
  bool scene_changed{false};
  LightType light_type{LightType::Directional};
};
}  // namespace ezg::gl
#endif  //RENDER_OPTION_HPP
