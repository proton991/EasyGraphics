#include "basic_renderer.hpp"
#include "render_api.hpp"
#include "scene.hpp"
namespace ezg::gl {
BasicRenderer::BasicRenderer(const BasicRenderer::Config& config)
    : m_width(config.width), m_height(config.height) {
  compile_shaders(config.shader_program_infos);
  setup_ubos();
  setup_screen_quad();
  setup_framebuffers(m_width, m_height);
}

void BasicRenderer::compile_shaders(
    const std::vector<ShaderProgramCreateInfo>& shader_program_infos) {
  // build shaders
  for (const auto& info : shader_program_infos) {
    auto shader_program = ShaderProgramFactory::create_shader_program(info);
    m_shader_cache.try_emplace(info.name, std::move(shader_program.value()));
  }
}

void BasicRenderer::setup_ubos() {
  m_camera_ubo = UniformBuffer::Create(sizeof(CameraData), 0);
  m_model_ubo  = UniformBuffer::Create(sizeof(ModelData), 1);
}

void BasicRenderer::setup_screen_quad() {
  float quadVertices[] = {
      // positions   // texCoords
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
      1.0f, -1.0f,  1.0f, 0.0f,

      -1.0f,  1.0f,  0.0f, 1.0f,
      1.0f, -1.0f,  1.0f, 0.0f,
      1.0f,  1.0f,  1.0f, 1.0f
  };
  m_quad_vao = VertexArray::Create();
  m_quad_vao->bind();
  auto vbo = VertexBuffer::Create(sizeof(quadVertices), quadVertices);
  vbo->set_buffer_view({
      {"aPos", BufferDataType::Vec2f},
      {"aTexCoords", BufferDataType::Vec2f},
  });
  m_quad_vao->attach_vertex_buffer(vbo);
  m_quad_vao->unbind();
}

void BasicRenderer::setup_framebuffers(uint32_t width, uint32_t height) {
  RenderTargetInfo rt_info;
  rt_info.height                 = height;
  rt_info.width                  = width;
  rt_info.samples                = 1;
  rt_info.has_depth              = true;
  rt_info.color_attachment_infos = {RTAttachmentFormat::RGBA8};
  m_gbuffer                      = RenderTarget::Create(rt_info);
}

void BasicRenderer::render_model(const ModelPtr& model, ShaderProgram& shader_program) {
  for (const auto& mesh : model->get_meshes()) {
    shader_program.set_uniform("u_model", mesh.model_matrix);
    RenderAPI::draw_mesh(mesh);
  }
}

void BasicRenderer::render_model(const ModelPtr& model) {
  for (const auto& mesh : model->get_meshes()) {
    mesh.material.bind_all_textures();
    m_model_data.model_matrix = mesh.model_matrix;
    m_model_ubo->set_data(&m_model_data.model_matrix, sizeof(ModelData));
    RenderAPI::draw_mesh(mesh);
  }
}

void BasicRenderer::update(const system::Camera& camera) {
  // camera ubo
  m_camera_data.view       = camera.get_view_matrix();
  m_camera_data.projection = camera.get_projection_matrix();
  m_camera_data.proj_view  = m_camera_data.projection * m_camera_data.view;
  m_camera_ubo->set_data(&m_camera_data.proj_view, sizeof(CameraData),
                         offsetof(CameraData, proj_view));
  m_camera_ubo->set_data(&m_camera_data.view, sizeof(CameraData), offsetof(CameraData, view));
  m_camera_ubo->set_data(&m_camera_data.projection, sizeof(CameraData),
                         offsetof(CameraData, projection));
}

void BasicRenderer::render_frame(const FrameInfo& info) {
  m_gbuffer->bind();
  RenderAPI::enable_depth_testing();
  RenderAPI::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f});
  RenderAPI::clear_color_and_depth();
  // forward pass
  auto& forward_shader = m_shader_cache.at("forward");
  forward_shader.use();

  update(info.camera);
  // render models
  for (const auto& model : info.scene->m_models) {
    for (const auto& mesh : model->get_meshes()) {

      forward_shader.set_uniform("u_baseColorFactor", mesh.material.base_color_factor);
      // bind textures
      mesh.material.bind_all_textures();
      // model ubo
      m_model_data.model_matrix = mesh.model_matrix;
      m_model_ubo->set_data(&m_model_data, sizeof(ModelData));

      RenderAPI::draw_mesh(mesh);
    }
    m_gbuffer->unbind();

    RenderAPI::disable_depth_testing();
    RenderAPI::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f});
    RenderAPI::clear_color();

    auto& screen_shader = m_shader_cache.at("screen");
    screen_shader.use();
    RenderAPI::draw_vertices(m_quad_vao, 6);
  }
}
}  // namespace ezg::gl