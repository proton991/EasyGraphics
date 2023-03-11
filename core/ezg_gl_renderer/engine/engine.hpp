#ifndef EASYGRAPHICS_ENGINE_HPP
#define EASYGRAPHICS_ENGINE_HPP
#include <string>
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
class SimpleScene;

class Engine {
public:
  Engine() = default;

  void initialize();

  void run();

private:
  void reload_scene(uint32_t index);

  Ref<system::StopWatch> m_stop_watch;
  Ref<system::Window> m_window;
  Ref<system::GUISystem> m_gui;
  Ref<SimpleScene> m_scene;

  Ref<system::Camera> m_camera;

  Ref<BasicRenderer> m_renderer;

  Ref<RenderOptions> m_options;

  int m_current_model_index{0};


  std::vector<const char*> ModelNames = {"ToyCar", "MetalRoughSpheres", "DamagedHelmet",
                                         "EnvironmentTest", "Sponza"};

  std::vector<const char*> ModelPaths = {
      "../../glTF-Sample-Models/2.0/ToyCar/glTF/ToyCar.gltf",
      "../../glTF-Sample-Models/2.0/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf",
      "../../glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf",
      "../../glTF-Sample-Models/2.0/EnvironmentTest/glTF/EnvironmentTest.gltf",
      "../../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf"};
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_ENGINE_HPP
