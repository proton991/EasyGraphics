#include "assets/model.hpp"
#include "assets/texture.hpp"
#include "graphics/shader.hpp"
#include "managers/resource_manager.hpp"
#include "renderer/basic_renderer.hpp"
#include "renderer/render_api.hpp"
#include "systems/camera_system.hpp"
#include "systems/profile_system.hpp"
#include "systems/window_system.hpp"
#include "graphics/render_target.hpp"

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
  std::vector<ShaderStage> stages1 = {
      {"../resources/shaders/using_framebuffer/model_loading.vs.glsl", "vertex"},
      {"../resources/shaders/using_framebuffer/model_loading.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo info1 {"ModelShader", stages1};

  std::vector<ShaderStage> stages2 = {
      {"../resources/shaders/using_framebuffer/framebuffers_screen.vs.glsl", "vertex"},
      {"../resources/shaders/using_framebuffer/framebuffers_screen.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo info2 {"ModelShader", stages2};
  auto shader_program = ShaderProgramFactory::create_shader_program(info1);
  auto screen_shader = ShaderProgramFactory::create_shader_program(info2);

  if (!shader_program.has_value() || !screen_shader.has_value()) {
    spdlog::error("Failed to create shader program!");
    std::abort();
  }

  screen_shader->use();
  screen_shader->set_uniform("screenTexture", 0);
  // setup screen quad vao
  // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
  float quadVertices[] = {
      // positions   // texCoords
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
      1.0f, -1.0f,  1.0f, 0.0f,

      -1.0f,  1.0f,  0.0f, 1.0f,
      1.0f, -1.0f,  1.0f, 0.0f,
      1.0f,  1.0f,  1.0f, 1.0f
  };

  auto quad_vao = VertexArrayObject::Create();
  quad_vao->bind();
  auto vbo = VertexBuffer::Create(sizeof(quadVertices), quadVertices);
  vbo->set_buffer_view({
      {"aPos", BufferDataType::Vec2f},
      {"aTexCoords", BufferDataType::Vec2f},
  });
  quad_vao->attach_vertex_buffer(vbo);

  auto helmet = ResourceManager::GetInstance().load_gltf_model("helmet", "../resources/models/FlightHelmet/FlightHelmet.gltf");
  //  auto helmet = ResourceManager::GetInstance().load_gltf_model("helmet", "C:/Dev/Code/ComputerGraphics/glTF-Sample-Models-master/2.0/ToyCar/glTF/ToyCar.gltf");
//    auto helmet = ResourceManager::GetInstance().load_gltf_model("helmet", "C:/Dev/Code/ComputerGraphics/glTF-Sample-Models-master/2.0/Sponza/glTF/Sponza.gltf");

  auto camera = Camera::CreateBasedOnBBox(helmet->get_aabb().bbx_min, helmet->get_aabb().bbx_max);

  RenderTargetInfo rt_info;
  rt_info.height = config.height;
  rt_info.width = config.width;
  rt_info.samples = 1;
  rt_info.has_depth = true;
  rt_info.color_attachment_infos = {RTAttachmentFormat::RGBA8};
  auto g_buffer = RenderTarget::Create(rt_info);

  StopWatch stop_watch;

  BasicRenderer renderer;

  while (!window.should_close()) {
    g_buffer->bind();
    RenderAPI::enable_depth_testing();

    RenderAPI::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f});
    RenderAPI::clear_color_and_depth();

    shader_program->use();
    shader_program->set_uniform("u_view", camera.GetViewMatrix());
    shader_program->set_uniform("u_projection", camera.GetProjectionMatrix());

    renderer.render_model(helmet, shader_program.value());

    g_buffer->unbind();
    RenderAPI::disable_depth_testing();
    RenderAPI::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f});
    RenderAPI::clear_color();

    screen_shader->use();

    RenderAPI::draw_vertices(quad_vao, 6);

    camera.Update(stop_watch.time_step());
    window.swap_buffers();
    window.update();
  }
  return 0;
}
