#include "skybox.hpp"
#include "graphics/framebuffer.hpp"
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
    // Back
    0, 1, 5,  //
    5, 4, 0,
    // Front
    3, 7, 6,  //
    6, 2, 3,  //
    // Left
    0, 4, 7,  //
    7, 3, 0,  //
    // Right
    1, 2, 6,  //
    6, 5, 1,  //
    // Bottom
    0, 3, 2,  //
    2, 1, 0,  //
    // Top
    4, 5, 6,  //
    6, 7, 4,  //
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
  std::vector<ShaderProgramCreateInfo> shader_program_infos;
  if (m_type == SkyboxType::Cubemap) {
    ShaderProgramCreateInfo info{
        "skybox",
        {
            {"../resources/shaders/simple_renderer/skybox.vs.glsl", "vertex"},
            {"../resources/shaders/simple_renderer/cubemap.fs.glsl", "fragment"},
        }};
    shader_program_infos.push_back(info);
  } else {
    ShaderProgramCreateInfo info1{
        "equirectangular_converter",
        {
            {"../resources/shaders/simple_renderer/skybox.vs.glsl", "vertex"},
            {"../resources/shaders/simple_renderer/equirectangular_converter.fs.glsl", "fragment"},
        }};
    ShaderProgramCreateInfo info2{
        "skybox",
        {
            {"../resources/shaders/simple_renderer/skybox.vs.glsl", "vertex"},
            {"../resources/shaders/simple_renderer/equirectangular.fs.glsl", "fragment"},
        }};
    shader_program_infos.push_back(info1);
    shader_program_infos.push_back(info2);
  }
  for (const auto& info : shader_program_infos) {
    auto shader_program = ShaderProgramFactory::create_shader_program(info);
    m_shader_cache.try_emplace(info.name, std::move(shader_program));
  }
}
Ref<Skybox> Skybox::Create(const std::vector<std::string>& face_paths) {
  return CreateRef<Skybox>(face_paths);
}

Ref<Skybox> Skybox::Create(const std::string& hdr_path, int resolution) {
  return CreateRef<Skybox>(hdr_path, resolution);
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
  auto ibo =
      IndexBuffer::Create(sizeof(SKY_BOX_INDICES) / sizeof(SKY_BOX_INDICES[0]), SKY_BOX_INDICES);
  m_cube_vao->attach_vertex_buffer(vbo);
  m_cube_vao->attach_index_buffer(ibo);
}

Skybox::Skybox(const std::vector<std::string>& face_paths) {
  m_type = SkyboxType::Cubemap;
  setup_shaders();
  setup_cube_quads();
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  m_cube_texture = ResourceManager::GetInstance().load_cubemap_textures("skybox1", face_paths);
}

Skybox::Skybox(const std::string& hdr_path, int resolution) {
  m_type = SkyboxType::Equirectangular;
  setup_shaders();
  setup_cube_quads();
  AttachmentInfo env_map_info{.width   = resolution,
                              .height  = resolution,
                              .type    = AttachmentType::TEXTURE_CUBEMAP,
                              .binding = AttachmentBinding::COLOR0,
                              .name    = "color",
                              // use float for hdr
                              .internal_format = GL_RGB16F,
                              .data_format     = GL_RGB,
                              .data_type       = GL_FLOAT};
  AttachmentInfo depth_stencil_info = AttachmentInfo::DepthStencil();
  FramebufferCreatInfo env_fbo_ci{.width             = static_cast<uint32_t>(resolution),
                                  .height            = static_cast<uint32_t>(resolution),
                                  .attachments_infos = {env_map_info, depth_stencil_info}};
  m_env_fbo = Framebuffer::Create(env_fbo_ci);

  auto hdr_texture = ResourceManager::GetInstance().load_hdr_texture(hdr_path);
  // Setup projection and view matrices for capturing data onto the 6 cubemap face directions
  auto capture_proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
  const glm::mat4 capture_views[]{
      glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
      glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
      glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};
  auto& convert_shader = m_shader_cache.at("equirectangular_converter");
  convert_shader->use();
  hdr_texture->bind(0);
  m_env_fbo->bind();
  for (int i = 0; i < 6; i++) {
    convert_shader->set_uniform("uProjView", capture_proj * capture_views[i]);
    m_env_fbo->attach_layer_texture(i, "color");
    RenderAPI::draw_indices(m_cube_vao);
  }
  m_env_fbo->unbind();
}

void Skybox::draw(const system::Camera& camera) {
  auto& skybox_shader = m_shader_cache.at("skybox");
  skybox_shader->use();
  if (m_type == SkyboxType::Cubemap) {
    m_cube_texture->bind(0);
  } else {
    m_env_fbo->bind_texture("color");
  }
  auto view = glm::mat4(glm::mat3(camera.get_view_matrix()));
  skybox_shader->set_uniform("uProjView", camera.get_projection_matrix() * view);
  RenderAPI::draw_indices(m_cube_vao);
}
}  // namespace ezg::gl