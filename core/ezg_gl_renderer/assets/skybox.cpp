#include "skybox.hpp"
#include <stb_image.h>
#include <iostream>
#include "managers/resource_manager.hpp"
#include "renderer/render_api.hpp"

namespace ezg::gl {
const float SKY_BOX_VERTICES[] = {
    //   Coordinates
    -1.0f, -1.0f, 1.0f,   //        7--------6
    1.0f,  -1.0f, 1.0f,   //       /|       /|
    1.0f,  -1.0f, -1.0f,  //      4--------5 |
    -1.0f, -1.0f, -1.0f,  //      | |      | |
    -1.0f, 1.0f,  1.0f,   //      | 3------|-2
    1.0f,  1.0f,  1.0f,   //      |/       |/
    1.0f,  1.0f,  -1.0f,  //      0--------1
    -1.0f, 1.0f,  -1.0f};

const unsigned int SKY_BOX_INDICES[] = {
    // Right
    1, 2, 6,  //
    6, 5, 1,  //
    // Left
    0, 4, 7,  //
    7, 3, 0,  //
    // Top
    4, 5, 6,  //
    6, 7, 4,  //
    // Bottom
    0, 3, 2,  //
    2, 1, 0,  //
    // Back
    0, 1, 5,  //
    5, 4, 0,
    // Front
    3, 7, 6,  //
    6, 2, 3   //
};

const float QUAD_VERTICES[] = {
    // positions   // texCoords
    -1.0f, 1.0f,  0.0f, 1.0f,  //
    -1.0f, -1.0f, 0.0f, 0.0f,  //
    1.0f,  -1.0f, 1.0f, 0.0f,  //

    -1.0f, 1.0f,  0.0f, 1.0f,  //
    1.0f,  -1.0f, 1.0f, 0.0f,  //
    1.0f,  1.0f,  1.0f, 1.0f   //
};

void Skybox::setup_shaders() {
  std::vector<ShaderStage> stages1 = {
      {"../resources/shaders/simple_renderer/cubemap.vs.glsl", "vertex"},
      {"../resources/shaders/simple_renderer/cubemap.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo info1{"skybox", stages1};
  std::vector<ShaderProgramCreateInfo> shader_program_infos = {info1};

  for (const auto& info : shader_program_infos) {
    auto shader_program = ShaderProgramFactory::create_shader_program(info);
    m_shader_cache.try_emplace(info.name, std::move(shader_program.value()));
  }


}
Ref<Skybox> Skybox::Create(const std::vector<std::string>& face_paths) {
  return CreateRef<Skybox>(face_paths);
}

void Skybox::setup_screen_quads() {

  m_quad_vao = VertexArray::Create();
  m_quad_vao->bind();
  auto vbo = VertexBuffer::Create(sizeof(QUAD_VERTICES), QUAD_VERTICES);
  vbo->set_buffer_view({
      {"aPos", BufferDataType::Vec2f},
      {"aTexCoords", BufferDataType::Vec2f},
  });
  m_quad_vao->attach_vertex_buffer(vbo);
  m_quad_vao->unbind();
}

void Skybox::setup_cube_quads() {
  m_cube_vao = VertexArray::Create();
  m_cube_vao->bind();
  auto vbo = VertexBuffer::Create(sizeof(SKY_BOX_VERTICES), SKY_BOX_VERTICES);
  vbo->set_buffer_view({
      {"aPos", BufferDataType::Vec3f},
  });
  auto ibo = IndexBuffer::Create(sizeof(SKY_BOX_INDICES)/sizeof(SKY_BOX_INDICES[0]), SKY_BOX_INDICES);
  m_cube_vao->attach_vertex_buffer(vbo);
  m_cube_vao->attach_index_buffer(ibo);
}

void Skybox::setup_cubemap_textures(const std::vector<std::string>& face_paths) {
  m_cube_texture =  ResourceManager::GetInstance().load_cubemap_textures("skybox1", face_paths);
}

Skybox::Skybox(const std::vector<std::string>& face_paths) : m_resolution{0}{
  setup_shaders();
  setup_cube_quads();
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  setup_cubemap_textures(face_paths);

}

void Skybox::draw(const system::Camera& camera) {
  auto& skybox_shader = m_shader_cache.at("skybox");
  skybox_shader.use();
  m_cube_texture->bind(0);
  auto view = glm::mat4(glm::mat3(camera.get_view_matrix()));
  skybox_shader.set_uniform("uViewProj", camera.get_projection_matrix() * view);
  RenderAPI::draw_indices(m_cube_vao);
}
}  // namespace ezg::gl