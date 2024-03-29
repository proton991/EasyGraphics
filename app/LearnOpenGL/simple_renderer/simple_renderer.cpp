#include "custom_scene.hpp"
#include "managers/resource_manager.hpp"
#include "renderer/basic_renderer.hpp"
#include "systems/gui_system.hpp"
#include "systems/profile_system.hpp"
#include "systems/window_system.hpp"

using namespace ezg::system;
using namespace ezg::gl;
using namespace ezg::app;
// choose discrete GPU, (Windows NVIDIA)
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

int main() {
  WindowConfig config{};
  config.width         = 800;
  config.height        = 800;
  config.major_version = 4;
  config.minor_version = 5;
  config.resizable     = GL_FALSE;
  config.title         = "OpenGL Renderer";
  Window window{config};
  std::vector<ShaderStage> stages1 = {
      {"../resources/shaders/simple_renderer/forward.vs.glsl", "vertex"},
      {"../resources/shaders/simple_renderer/pbr_cook_torrance.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo info1{"pbr", stages1};

  std::vector<ShaderStage> stages2 = {
      {"../resources/shaders/simple_renderer/framebuffers_screen.vs.glsl", "vertex"},
      {"../resources/shaders/simple_renderer/framebuffers_screen.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo info2{"screen", stages2};

  std::vector<std::string> model_paths{
      "../../glTF-Sample-Models/2.0/ToyCar/glTF/ToyCar.gltf",
      "../../glTF-Sample-Models/2.0/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf",
      "../../glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf",
      "../../glTF-Sample-Models/2.0/EnvironmentTest/glTF/EnvironmentTest.gltf"};

  ResourceManager::GetInstance().load_gltf_model(model_paths[0]);

  auto scene = SceneBuilder::Create<CustomScene>("demo");
  scene->init();

  auto camera = Camera::CreateBasedOnBBox(scene->get_aabb().bbx_min, scene->get_aabb().bbx_max);

  StopWatch stop_watch;
  RendererConfig render_config{
      config.width,
      config.height
  };
  BasicRenderer renderer{render_config};

  while (!window.should_close()) {
    FrameInfo frame_info{scene, camera};

    frame_info.scene->update();

    renderer.render_frame(frame_info);

    camera.update(stop_watch.time_step());

    window.swap_buffers();
    window.update();
  }
  return 0;
}
