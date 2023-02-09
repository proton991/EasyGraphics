#include <iostream>
#include "systems/window_system.hpp"
#include "graphics/shader.hpp"
#include "assets/model.hpp"
#include "renderer/render_api.hpp"

using namespace ezg::system;
using namespace ezg::gl;
int main()
{
  Window window{Window::default_config()};
  std::vector<ShaderStage> stages = {
      {"../resources/shaders/hello_triangle.vs", "vertex"},
      {"../resources/shaders/hello_triangle.fs", "fragment"},
  };
  ShaderProgramCreateInfo shader_program_info {"hello triangle", stages};
  auto shader_program = ShaderProgramFactory::create_shader_program(shader_program_info);
  if (!shader_program.has_value()) {
    spdlog::error("Failed to create shader program!");
    std::abort();
  }
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float vertices[] = {
      // positions         // colors
      0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
      0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top
  };
  auto vbo = VertexBuffer::Create(sizeof(vertices), vertices);
  vbo->set_buffer_view({
      {"aPos", BufferDataType::Vec3f},
      {"aColor", BufferDataType::Vec3f}
  });
  auto vao = VertexArrayObject::Create();
  vao->attach_vertex_buffer(vbo);

  while (!window.should_close()) {
    RenderAPI::set_clear_color({0.2f, 0.3f, 0.3f, 1.0f});
    RenderAPI::clear_color_and_depth();
    shader_program->use();
    RenderAPI::draw_vertices(vao, 3);

    window.swap_buffers();
    window.update();
  }
  return 0;
}
