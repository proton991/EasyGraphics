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
      {"../shaders/gl/hello_triangle.vs", "vertex"},
      {"../shaders/gl/hello_triangle.fs", "fragment"},
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

  std::shared_ptr<BaseVAO> vao = std::make_shared<SimpleVAO>();
  vao->bind();
  vao->attach_buffer(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  vao->enable_attribute(0, 3, 6 * sizeof(float), 0);
  vao->enable_attribute(1, 3, 6 * sizeof(float), 3 * sizeof(float));

  while (!window.should_close()) {
    RenderAPI::set_clear_color({0.2f, 0.3f, 0.3f, 1.0f});
    RenderAPI::clear();
    shader_program->use();
    RenderAPI::draw_vertices(vao, 3);

    window.swap_buffers();
    window.update();
  }
  return 0;
}
