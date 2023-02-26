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
  if (!shader_program) {
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
  uint32_t indices[] = {  // note that we start from 0!
      0, 1, 3,  // first Triangle
      1, 2, 3   // second Triangle
  };

  auto vbo = VertexBuffer::Create(sizeof(vertices), vertices);
  vbo->set_buffer_view({
      {"aPos", BufferDataType::Vec3f},
      {"aTexCoords", BufferDataType::Vec2f}
  });
  auto ibo = IndexBuffer::Create(sizeof(indices)/sizeof(uint32_t), indices);
  auto vao = VertexArray::Create();
  vao->attach_vertex_buffer(vbo);
  vao->attach_index_buffer(ibo);

  // load texture
  auto container_texture = Texture2D::Create("../resources/textures/container.jpg");
  while (!window.should_close()) {
    RenderAPI::set_clear_color({0.2f, 0.3f, 0.3f, 1.0f});
    RenderAPI::clear_color_and_depth();

    // bind texture
    container_texture->bind(0);

    shader_program->use();

    RenderAPI::draw_indices(vao);
    //    RenderAPI::draw_vertices(vao, 4);
    window.swap_buffers();
    window.update();
  }
  return 0;
}
