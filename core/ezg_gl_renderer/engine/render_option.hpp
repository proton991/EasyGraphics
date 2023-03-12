#ifndef RENDER_OPTION_HPP
#define RENDER_OPTION_HPP
namespace ezg::gl {
struct RenderOptions {
  RenderOptions() = default;
  const char** model_list{nullptr};
  uint32_t num_models{0};
  int selected_model{0};
  bool rotate_model{false};
  bool rotate_camera{false};
  bool enable_env_map{true};
  bool show_bg{true};
  bool show_axis{false};
  bool show_aabb{false};
  bool blur{false};
  bool scene_changed{false};
};
}  // namespace ezg::gl
#endif  //RENDER_OPTION_HPP
