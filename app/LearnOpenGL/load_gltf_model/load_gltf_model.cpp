#include "assets/model.hpp"
#include "assets/texture.hpp"
#include "graphics/shader.hpp"
#include "managers/resource_manager.hpp"
#include "renderer/basic_renderer.hpp"
#include "renderer/render_api.hpp"
#include "systems/camera_system.hpp"
#include "systems/profile_system.hpp"
#include "systems/window_system.hpp"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
using namespace ezg::system;
using namespace ezg::gl;
int main()
{
  WindowConfig config{};
  config.width         = 800;
  config.height        = 800;
  config.major_version = 4;
  config.minor_version = 5;
  config.resizable     = GL_FALSE;
  config.title         = "OpenGL Renderer";
  Window window{config};
  std::vector<ShaderStage> stages = {
      {"../resources/shaders/model_loading.vs.glsl", "vertex"},
      {"../resources/shaders/model_loading.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo shader_program_info {"ModelShader", stages};
  auto shader_program = ShaderProgramFactory::create_shader_program(shader_program_info);
  if (!shader_program.has_value()) {
    spdlog::error("Failed to create shader program!");
    std::abort();
  }

  auto helmet = ResourceManager::GetInstance().load_gltf_model("helmet", "../resources/models/FlightHelmet/FlightHelmet.gltf");
//  auto helmet = ResourceManager::GetInstance().load_gltf_model("helmet", "C:/Dev/Code/ComputerGraphics/glTF-Sample-Models-master/2.0/ToyCar/glTF/ToyCar.gltf");
//  auto helmet = ResourceManager::GetInstance().load_gltf_model("helmet", "C:/Dev/Code/ComputerGraphics/glTF-Sample-Models-master/2.0/Sponza/glTF/Sponza.gltf");

  auto camera = Camera::CreateBasedOnBBox(helmet->get_aabb().bbx_min, helmet->get_aabb().bbx_max);

  StopWatch stop_watch;
  // render the loaded model
  glm::mat4 model_mat = glm::mat4(1.0f);
  model_mat = glm::translate(model_mat, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
  model_mat = glm::rotate(model_mat, glm::radians(180.0f), glm::vec3(0,1,0));
  model_mat = glm::scale(model_mat, glm::vec3(1.2f, 1.2f, 1.2f));
  BasicRenderer renderer;
  RenderAPI::enable_depth_testing();
  while (!window.should_close()) {
    RenderAPI::set_clear_color({0.2f, 0.3f, 0.3f, 1.0f});
    RenderAPI::clear_color_and_depth();

    camera.update(stop_watch.time_step());

    shader_program->use();
//    shader_program->set_uniform("u_model", model_mat);
    shader_program->set_uniform("u_view", camera.get_view_matrix());
    shader_program->set_uniform("u_projection", camera.get_projection_matrix());

    renderer.render_model(helmet, shader_program.value());
    window.swap_buffers();
    window.update();
  }
  return 0;
}
