#include "systems/window_system.hpp"
#include "graphics/shader.hpp"
#include "assets/texture.hpp"
#include "assets/model.hpp"
#include "renderer/render_api.hpp"

using namespace ezg::system;
using namespace ezg::gl;
int main()
{
  WindowConfig config{};
  config.width         = 800;
  config.height        = 600;
  config.major_version = 4;
  config.minor_version = 5;
  config.resizable     = GL_FALSE;
  config.title         = "OpenGL Renderer";
  Window window{config};
  std::vector<ShaderStage> stages = {
      {"../resources/shaders/hello_triangle_textured.vs.glsl", "vertex"},
      {"../resources/shaders/hello_triangle_textured.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo shader_program_info {"hello triangle", stages};
  auto shader_program = ShaderProgramFactory::create_shader_program(shader_program_info);
  if (!shader_program.has_value()) {
    spdlog::error("Failed to create shader program!");
    std::abort();
  }
  shader_program->set_uniform("texture1", 0);
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float vertices[] = {
      // positions          // texture coords
      0.5f,   0.5f, 0.0f,   1.0f, 1.0f, // top right
      0.5f,  -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
      -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
      -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left
  };
  unsigned int indices[] = {  // note that we start from 0!
      0, 1, 3,  // first Triangle
      1, 2, 3   // second Triangle
  };

  auto vao = SimpleIndexVAO::create();
  //  std::shared_ptr<SimpleIndexVAO> vao = std::make_shared<SimpleIndexVAO>();
  vao->bind();
  vao->attach_buffer(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  vao->attach_buffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  vao->enable_attribute(0, 3, 5 * sizeof(float), 0);
  vao->enable_attribute(1, 2, 5 * sizeof(float), 3 * sizeof(float));

  // load texture
  auto container_texture = Texture2D::create("../resources/textures/container.jpg");
  while (!window.should_close()) {
    RenderAPI::set_clear_color({0.2f, 0.3f, 0.3f, 1.0f});
    RenderAPI::clear();

    // bind texture
    container_texture->bind(0);

    shader_program->use();

    RenderAPI::draw_indices(vao, 6);

    window.swap_buffers();
    window.update();
  }
  return 0;
}
