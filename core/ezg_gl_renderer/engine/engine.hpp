#ifndef EASYGRAPHICS_ENGINE_HPP
#define EASYGRAPHICS_ENGINE_HPP
#include <string>
#include <unordered_map>
#include <vector>
#include "base.hpp"
#include "render_option.hpp"

namespace ezg::system {
class Window;
class GUISystem;
class Camera;
class StopWatch;
}  // namespace ezg::system

namespace ezg::gl {
class BasicRenderer;
class BaseScene;

class Engine {
public:
  Engine() = default;

  void initialize(const std::string& active_scene);

  void run();

private:
  void load_scene(uint32_t index);

  Ref<system::StopWatch> m_stop_watch;
  Ref<system::Window> m_window;
  Ref<system::GUISystem> m_gui;
  Ref<BaseScene> m_scene;
  std::unordered_map<std::string, Ref<BaseScene>> m_scene_cache;

  Ref<system::Camera> m_camera;

  Ref<BasicRenderer> m_renderer;

  Ref<RenderOptions> m_options;
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_ENGINE_HPP
