#include "assets/model.hpp"
#include "assets/texture.hpp"
#include "graphics/shader.hpp"
#include "renderer/render_api.hpp"
#include "systems/camera_system.hpp"
#include "systems/profile_system.hpp"
#include "systems/window_system.hpp"

using namespace ezg::gl;
using namespace ezg::system;
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
  RenderAPI::enable_depth_testing();
  std::vector<ShaderStage> stages = {
      {"../resources/shaders/camera_class.vs.glsl", "vertex"},
      {"../resources/shaders/camera_class.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo shader_program_info {"ModelShader", stages};
  auto shader_program = ShaderProgramFactory::create_shader_program(shader_program_info);
  if (!shader_program) {
    spdlog::error("Failed to create shader program!");
    std::abort();
  }
  float vertices[] = {
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
      0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
  };
  // world space positions of our cubes
  glm::vec3 cubePositions[] = {
      glm::vec3( 0.0f,  0.0f,  0.0f),
      glm::vec3( 2.0f,  5.0f, 15.0f),
      glm::vec3(-1.5f, -2.2f, 2.5f),
      glm::vec3(-3.8f, -2.0f, 12.3f),
      glm::vec3( 2.4f, -0.4f, 3.5f),
      glm::vec3(-1.7f,  3.0f, 7.5f),
      glm::vec3( 1.3f, -2.0f, 2.5f),
      glm::vec3( 1.5f,  2.0f, 2.5f),
      glm::vec3( 1.5f,  0.2f, 1.5f),
      glm::vec3(-1.3f,  1.0f, 1.5f)
  };
  auto vbo = VertexBuffer::Create(sizeof(vertices), vertices);
  vbo->set_buffer_view({
      {"aPos", BufferDataType::Vec3f},
      {"aTexCoords", BufferDataType::Vec2f}
  });
  auto vao = VertexArray::Create();
  vao->attach_vertex_buffer(vbo);

  // load texture
  auto container_texture = Texture2D::Create("../resources/textures/container.jpg");
  auto face_texture = Texture2D::Create("../resources/textures/awesomeface.png");

  shader_program->set_uniform("texture1", 0);
  shader_program->set_uniform("texture2", 1);

  auto camera = Camera::CreateDefault();

  StopWatch stop_watch;


//  BasicRenderer renderer;
  while (!window.should_close()) {
    RenderAPI::set_clear_color({0.2f, 0.3f, 0.3f, 1.0f});
    RenderAPI::clear_color_and_depth();

    // mix 2 textures
    container_texture->bind(0);
    face_texture->bind(1);

    shader_program->use();
    // render boxes

    for (unsigned int i = 0; i < 10; i++)
    {
      // render the loaded model
      glm::mat4 model_mat = glm::mat4(1.0f);
      // calculate the model matrix for each object and pass it to shader before drawing
      model_mat = glm::translate(model_mat, cubePositions[i]);
      float angle = 20.0f * i;
      model_mat = glm::rotate(model_mat, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
      shader_program->set_uniform("u_model", model_mat);

      RenderAPI::draw_vertices(vao, 36);
    }
    shader_program->set_uniform("u_view", camera.get_view_matrix());
    shader_program->set_uniform("u_projection", camera.get_projection_matrix());

    camera.update(stop_watch.time_step());
    window.swap_buffers();
    window.update();
  }
  return 0;
}
