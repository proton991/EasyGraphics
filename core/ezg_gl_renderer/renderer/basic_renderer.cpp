#include "basic_renderer.hpp"

#include <memory>
#include "assets/skybox.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/shader.hpp"
#include "render_api.hpp"
#include "scene.hpp"

namespace ezg::gl {
BasicRenderer::BasicRenderer(const RendererConfig& config)
    : m_width(config.width), m_height(config.height) {
  ShaderProgramCreateInfo info1{
      "pbr",
      {
          {"../resources/shaders/simple_renderer/forward.vs.glsl", "vertex"},
          {"../resources/shaders/simple_renderer/pbr_cook_torrance.fs.glsl", "fragment"},
      }};

  ShaderProgramCreateInfo info2{
      "screen",
      {
          {"../resources/shaders/simple_renderer/framebuffers_screen.vs.glsl", "vertex"},
          {"../resources/shaders/simple_renderer/framebuffers_screen.fs.glsl", "fragment"},
      }};

  compile_shaders({info1, info2});
  setup_ubos();
  setup_screen_quad();
  setup_framebuffers(m_width, m_height);
  setup_coordinate_axis();
  setup_skybox();
}

void BasicRenderer::compile_shaders(
    const std::vector<ShaderProgramCreateInfo>& shader_program_infos) {
  // build shaders
  for (const auto& info : shader_program_infos) {
    auto shader_program = ShaderProgramFactory::create_shader_program(info);
    m_shader_cache.try_emplace(info.name, std::move(shader_program));
  }
}

void BasicRenderer::setup_ubos() {
  m_camera_ubo      = UniformBuffer::Create(sizeof(CameraData), 0);
  m_model_ubo       = UniformBuffer::Create(sizeof(ModelData), 1);
  m_pbr_sampler_ubo = UniformBuffer::Create(sizeof(ModelData), 2);
}

void BasicRenderer::setup_screen_quad() {
  float quadVertices[] = {
      // positions   // texCoords
      -1.0f, 1.0f,  0.0f, 1.0f,  //
      -1.0f, -1.0f, 0.0f, 0.0f,  //
      1.0f,  -1.0f, 1.0f, 0.0f,  //

      -1.0f, 1.0f,  0.0f, 1.0f,  //
      1.0f,  -1.0f, 1.0f, 0.0f,  //
      1.0f,  1.0f,  1.0f, 1.0f   //
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
  // srgb color attachment
  AttachmentInfo color_info =
      AttachmentInfo::Color("color", AttachmentBinding::COLOR0, width, height);
  color_info.internal_format = GL_SRGB8_ALPHA8;
  AttachmentInfo depth {AttachmentInfo::Depth(width, height)};
  std::vector<AttachmentInfo> attachment_infos{color_info, depth};
  FramebufferCreatInfo framebuffer_ci{width, height, attachment_infos};
  m_pbuffer = Framebuffer::Create(framebuffer_ci);
}

void BasicRenderer::setup_coordinate_axis() {
  std::vector<ShaderStage> stages = {
      {"../resources/shaders/simple_renderer/coords_axis.vs.glsl", "vertex"},
      {"../resources/shaders/simple_renderer/coords_axis.fs.glsl", "fragment"},
  };
  ShaderProgramCreateInfo info{"coords_axis", stages};
  m_shader_cache.try_emplace(info.name, ShaderProgramFactory::create_shader_program(info));
  m_axis_data.line_vertices = {
      // x axis
      {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
      {glm::vec3(100.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
      // y axis
      {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
      {glm::vec3(0.0f, 100.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
      // z axis
      {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
      {glm::vec3(0.0f, 0.0f, 100.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
  };
  auto vbo = VertexBuffer::Create(m_axis_data.line_vertices.size() * sizeof(LineVertex),
                                  m_axis_data.line_vertices.data());
  vbo->set_buffer_view({
      {"aPosition", BufferDataType::Vec3f},
      {"aColor", BufferDataType::Vec4f},
  });
  m_axis_data.vao = VertexArray::Create();
  m_axis_data.vao->bind();
  m_axis_data.vao->attach_vertex_buffer(vbo);
}

void BasicRenderer::setup_skybox() {
  std::vector<std::string> face_paths = {
      "../resources/textures/skybox/right.jpg",   //
      "../resources/textures/skybox/left.jpg",    //
      "../resources/textures/skybox/top.jpg",     //
      "../resources/textures/skybox/bottom.jpg",  //
      "../resources/textures/skybox/front.jpg",   //
      "../resources/textures/skybox/back.jpg",    //
  };
  //  m_skybox = Skybox::Create(face_paths);
  m_skybox = Skybox::Create("../resources/textures/hdri/barcelona.hdr", 2048);
}

void BasicRenderer::set_default_state() {
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_FRAMEBUFFER_SRGB);
  RenderAPI::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f});
  RenderAPI::enable_blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void BasicRenderer::update(const FrameInfo& info) {
  // camera ubo
  m_camera_data.view       = info.camera.get_view_matrix();
  m_camera_data.projection = info.camera.get_projection_matrix();
  m_camera_data.proj_view  = m_camera_data.projection * m_camera_data.view;

  m_camera_ubo->set_data(&m_camera_data, sizeof(CameraData));
  // forward pass
  auto& shader = m_shader_cache.at("pbr");
  shader->use();
  shader->set_uniform("uLightIntensity", info.scene->get_light_intensity());
  shader->set_uniform("uLightPos", info.scene->get_light_pos());
  shader->set_uniform("uLightDir", info.scene->get_light_dir());
  shader->set_uniform("uCameraPos", info.camera.get_pos());
}

void BasicRenderer::render_frame(const FrameInfo& info) {
  set_default_state();
  m_pbuffer->bind();
  RenderAPI::enable_depth_testing();
  RenderAPI::clear_color_and_depth();

  // bind Prefiltered IBL texture
  m_skybox->bind_prefilter_data();

  update(info);
  // render models
  for (const auto& model : info.scene->m_models) {
    for (const auto& mesh : model->get_meshes()) {
      m_sampler_data = {};
      // model ubo
      m_model_data.model_matrix = mesh.model_matrix;
      m_model_ubo->set_data(&m_model_data, sizeof(ModelData));
      // bindless textures
      mesh.material.upload_textures(m_shader_cache.at("pbr"), m_sampler_data);
      m_pbr_sampler_ubo->set_data(m_sampler_data.samplers, sizeof(PBRSamplerData));
      // draw mesh
      RenderAPI::draw_mesh(mesh);
    }
  }
  m_shader_cache.at("coords_axis")->use();
  RenderAPI::draw_line(m_axis_data.vao, 6);
  m_skybox->draw(info.camera);
  m_pbuffer->unbind();

  RenderAPI::disable_depth_testing();
  RenderAPI::clear_color();

  auto& screen_shader = m_shader_cache.at("screen");
  screen_shader->use();
  m_pbuffer->bind_texture("color", 0);
  RenderAPI::draw_vertices(m_quad_vao, 6);
}

void BasicRenderer::resize_fbos(int width, int height) {
  m_width  = width;
  m_height = height;
  setup_framebuffers(m_width, m_height);
  glViewport(0, 0, width, height);
}
}  // namespace ezg::gl