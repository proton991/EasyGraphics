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

// Setup projection and view matrices for capturing data onto the 6 cubemap face directions
const auto CaptureProjs = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
const glm::mat4 CaptureViews[]{
    glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

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
    ShaderProgramCreateInfo converter_info{
        "equirectangular_converter",
        {
            {"../resources/shaders/simple_renderer/skybox.vs.glsl", "vertex"},
            {"../resources/shaders/simple_renderer/equirectangular_converter.fs.glsl", "fragment"},
        }};
    ShaderProgramCreateInfo skybox_info{
        "skybox",
        {
            {"../resources/shaders/simple_renderer/skybox.vs.glsl", "vertex"},
            {"../resources/shaders/simple_renderer/equirectangular.fs.glsl", "fragment"},
        }};
    ShaderProgramCreateInfo prefilter_diffuse_info{
        "prefilter_diffuse",
        {
            {"../resources/shaders/simple_renderer/skybox.vs.glsl", "vertex"},
            {"../resources/shaders/simple_renderer/prefilter_diffuse.fs.glsl", "fragment"},
        }};
    ShaderProgramCreateInfo prefilter_specular_info{
        "prefilter_specular",
        {
            {"../resources/shaders/simple_renderer/skybox.vs.glsl", "vertex"},
            {"../resources/shaders/simple_renderer/prefilter_specular.fs.glsl", "fragment"},
        }};
    ShaderProgramCreateInfo brdf_lut_info{
        "brdf_integration",
        {
            {"../resources/shaders/simple_renderer/framebuffers_screen.vs.glsl", "vertex"},
            {"../resources/shaders/simple_renderer/brdf_integration.fs.glsl", "fragment"},
        }};
    shader_program_infos.push_back(converter_info);
    shader_program_infos.push_back(skybox_info);
    shader_program_infos.push_back(prefilter_diffuse_info);
    shader_program_infos.push_back(prefilter_specular_info);
    shader_program_infos.push_back(brdf_lut_info);
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

Skybox::Skybox(const std::string& hdr_path, int resolution) : m_resolution(resolution) {
  m_type = SkyboxType::Equirectangular;
  setup_shaders();
  setup_cube_quads();
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  AttachmentInfo base_color{.width   = resolution,
                            .height  = resolution,
                            .type    = AttachmentType::TEXTURE_CUBEMAP,
                            .binding = AttachmentBinding::COLOR0,
                            .name    = "base_color",
                            // use float for hdr
                            .internal_format = GL_RGB16F,
                            .data_format     = GL_RGB,
                            .data_type       = GL_FLOAT};
  FramebufferCreatInfo env_fbo_ci{.width             = static_cast<uint32_t>(resolution),
                                  .height            = static_cast<uint32_t>(resolution),
                                  .attachments_infos = {base_color}};
  m_env_fbo = Framebuffer::Create(env_fbo_ci);

  auto hdr_texture     = ResourceManager::GetInstance().load_hdr_texture(hdr_path);
  auto& convert_shader = m_shader_cache.at("equirectangular_converter");

  convert_shader->use();
  hdr_texture->bind(0);
  m_env_fbo->bind();
  for (int i = 0; i < 6; i++) {
    convert_shader->set_uniform("uProjView", CaptureProjs * CaptureViews[i]);
    m_env_fbo->attach_layer_texture(i, "base_color");
    draw_cube();
  }
  m_env_fbo->unbind();
  // calculate prefiltered IBL data
  calc_prefilter_diffuse();
  calc_prefilter_specular();
  calc_brdf_integration();
}

void Skybox::calc_prefilter_diffuse() {
  AttachmentInfo prefilter_diffuse{.width   = DIFFUSE_RESOLUTION,
                                   .height  = DIFFUSE_RESOLUTION,
                                   .type    = AttachmentType::TEXTURE_CUBEMAP,
                                   .binding = AttachmentBinding::COLOR0,
                                   .name    = "prefilter_diffuse",
                                   // use float for hdr
                                   .internal_format = GL_RGB16F,
                                   .data_format     = GL_RGB,
                                   .data_type       = GL_FLOAT};

  auto& prefilter_diffuse_shader = m_shader_cache.at("prefilter_diffuse");
  prefilter_diffuse_shader->use();
  m_env_fbo->bind(false);
  m_env_fbo->add_attachment(prefilter_diffuse);
  m_env_fbo->resize_depth_renderbuffer(DIFFUSE_RESOLUTION, DIFFUSE_RESOLUTION);
  m_env_fbo->bind_texture("base_color", 0);
  glViewport(0, 0, DIFFUSE_RESOLUTION, DIFFUSE_RESOLUTION);
  for (int i = 0; i < 6; i++) {
    prefilter_diffuse_shader->set_uniform("uProjView", CaptureProjs * CaptureViews[i]);
    m_env_fbo->attach_layer_texture(i, "prefilter_diffuse");
    draw_cube();
  }
  m_env_fbo->unbind();
}

void Skybox::calc_prefilter_specular() {
  AttachmentInfo prefilter_specular{.width   = SPECULAR_RESOLUTION,
                                    .height  = SPECULAR_RESOLUTION,
                                    .type    = AttachmentType::TEXTURE_CUBEMAP,
                                    .binding = AttachmentBinding::COLOR0,
                                    .name    = "prefilter_specular",
                                    // use float for hdr
                                    .internal_format = GL_RGB16F,
                                    .data_format     = GL_RGB,
                                    .data_type       = GL_FLOAT,
                                    .generate_mipmap = true};

  auto& shader = m_shader_cache.at("prefilter_specular");
  shader->use();
  m_env_fbo->bind(false);
  m_env_fbo->add_attachment(prefilter_specular);
  m_env_fbo->bind_texture("base_color", 0);
  unsigned int maxMipLevels = 5;
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    auto mipWidth  = static_cast<int>((SPECULAR_RESOLUTION)*std::pow(0.5, mip));
    auto mipHeight = static_cast<int>((SPECULAR_RESOLUTION)*std::pow(0.5, mip));
    m_env_fbo->resize_depth_renderbuffer(mipWidth, mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(maxMipLevels - 1);
    shader->set_uniform("uRoughness", roughness);
    for (auto i = 0; i < 6; ++i) {
      shader->set_uniform("uProjView", CaptureProjs * CaptureViews[i]);
      m_env_fbo->attach_layer_texture(i, "prefilter_specular");
      draw_cube();
    }
  }
  m_env_fbo->unbind();
}

void Skybox::calc_brdf_integration() {
  setup_screen_quads();
  AttachmentInfo info{.width   = m_resolution,
                      .height  = m_resolution,
                      .type    = AttachmentType::TEXTURE_2D,
                      .binding = AttachmentBinding::COLOR0,
                      .name    = "brdf_integration",
                      // use float for hdr
                      .internal_format = GL_RG16F,
                      .data_format     = GL_RG,
                      .data_type       = GL_FLOAT};
  FramebufferCreatInfo screen_fbo_ci{.width             = static_cast<uint32_t>(m_resolution),
                                     .height            = static_cast<uint32_t>(m_resolution),
                                     .attachments_infos = {info}};
  m_screen_fbo = Framebuffer::Create(screen_fbo_ci);
  auto& shader = m_shader_cache.at("brdf_integration");

  m_screen_fbo->bind();
  m_screen_fbo->clear();
  shader->use();
  draw_quad();
  m_screen_fbo->unbind();
}

void Skybox::bind_prefilter_data() {
  m_env_fbo->bind_texture("prefilter_diffuse", 3);
  m_env_fbo->bind_texture("prefilter_specular", 4);
  m_screen_fbo->bind_texture("brdf_integration", 5);
}

void Skybox::draw(const system::Camera& camera) {
  auto& skybox_shader = m_shader_cache.at("skybox");
  skybox_shader->use();
  if (m_type == SkyboxType::Cubemap) {
    m_cube_texture->bind(0);
  } else {
    m_env_fbo->bind_texture("base_color", 0);
  }
  auto view = glm::mat4(glm::mat3(camera.get_view_matrix()));
  skybox_shader->set_uniform("uProjView", camera.get_projection_matrix() * view);
  draw_cube();
}
void Skybox::draw_quad() {
  RenderAPI::draw_vertices(m_quad_vao, 6);
}

void Skybox::draw_cube() {
  RenderAPI::draw_indices(m_cube_vao);
}
}  // namespace ezg::gl