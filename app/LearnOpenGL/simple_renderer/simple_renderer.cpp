#include "custom_scene.hpp"
#include "managers/resource_manager.hpp"
#include "renderer/basic_renderer.hpp"
#include "systems/profile_system.hpp"
#include "systems/window_system.hpp"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
using namespace ezg::system;
using namespace ezg::gl;
using namespace ezg::app;
// choose discrete GPU, (Windows NVIDIA)
extern "C"
{
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
      {"../resources/shaders/simple_renderer/forward.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo info1{"forward", stages1};

  std::vector<ShaderStage> stages2 = {
      {"../resources/shaders/simple_renderer/framebuffers_screen.vs.glsl", "vertex"},
      {"../resources/shaders/simple_renderer/framebuffers_screen.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo info2{"screen", stages2};

  std::vector<std::string> model_paths{
      "../../glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf",
      "../../glTF-Sample-Models/2.0/ToyCar/glTF/ToyCar.gltf",
      "../../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf"};

  ResourceManager::GetInstance().load_gltf_model("helmet", model_paths[2]);

  auto scene = std::make_shared<CustomScene>("demo");
  scene->init();

  auto camera = Camera::CreateBasedOnBBox(scene->get_aabb().bbx_min, scene->get_aabb().bbx_max);

  StopWatch stop_watch;
  BasicRenderer::Config render_config{
      config.width,
      config.height,
      {info1, info2},
  };
  BasicRenderer renderer{render_config};

  while (!window.should_close()) {
    FrameInfo frame_info{scene, camera};

    renderer.render_frame(frame_info);

    camera.update(stop_watch.time_step());

    window.swap_buffers();
    window.update();
  }
  return 0;
}
