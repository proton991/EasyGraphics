#include "basic_renderer.hpp"

#include <memory>
#include "assets/line.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/shader.hpp"
#include "render_api.hpp"
#include "shadow_map.hpp"

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
  ShaderProgramCreateInfo info3{
      "lines",
      {
          {"../resources/shaders/simple_renderer/lines.vs.glsl", "vertex"},
          {"../resources/shaders/simple_renderer/lines.fs.glsl", "fragment"},
      }};
  m_shader_cache.try_emplace(info3.name, ShaderProgramFactory::create_shader_program(info3));
  compile_shaders({info1, info2, info3});
  setup_ubos();
  setup_screen_quad();
  setup_framebuffers(m_width, m_height);
  setup_coordinate_axis();
  m_aabb_line  = CreateRef<Line>();
  m_shadow_map = CreateRef<ShadowMap>(1024, 1024);
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
  AttachmentInfo depth{AttachmentInfo::Depth(width, height)};
  std::vector<AttachmentInfo> attachment_infos{color_info, depth};
  FramebufferCreatInfo framebuffer_ci{width, height, attachment_infos};
  m_pbuffer = Framebuffer::Create(framebuffer_ci);
}

void BasicRenderer::setup_coordinate_axis() {
  m_axis_line                = CreateRef<Line>();
  m_axis_line->line_vertices = {
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
  auto vbo = VertexBuffer::Create(m_axis_line->line_vertices.size() * sizeof(LineVertex),
                                  m_axis_line->line_vertices.data());
  vbo->set_buffer_view({
      {"aPosition", BufferDataType::Vec3f},
      {"aColor", BufferDataType::Vec4f},
  });
  m_axis_line->vao = VertexArray::Create();
  m_axis_line->vao->bind();
  m_axis_line->vao->attach_vertex_buffer(vbo);
}

void BasicRenderer::set_default_state() {
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_FRAMEBUFFER_SRGB);
  RenderAPI::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f});
  RenderAPI::enable_blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void BasicRenderer::update_ubo(const FrameInfo& info) {
  // camera ubo
  m_camera_data.view       = info.camera->get_view_matrix();
  m_camera_data.projection = info.camera->get_projection_matrix();
  m_camera_data.proj_view  = m_camera_data.projection * m_camera_data.view;
  m_camera_ubo->set_data(&m_camera_data, sizeof(CameraData));
  // scene ubo
  auto& shader = m_shader_cache.at("pbr");
  shader->use();
  shader->set_uniform("uLightIntensity", info.scene->get_light_intensity());
  shader->set_uniform("uLightPos", info.scene->get_light_pos());
  shader->set_uniform("uLightDir", info.scene->get_light_dir());
  shader->set_uniform("uLightType", static_cast<int>(info.options->light_type));
  shader->set_uniform("uCameraPos", info.camera->get_pos());
  shader->set_uniform("uLightSpaceMat", m_shadow_map->get_light_space_mat());
}

void BasicRenderer::render_meshes(const std::vector<Mesh>& meshes) {
  for (const auto& mesh : meshes) {
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

void BasicRenderer::render_scene(const FrameInfo& info) {
  m_shadow_map->bind_for_read(6);
  if (info.scene->has_skybox()) {
    // bind Prefiltered IBL texture
    if (info.options->enable_env_map) {
      info.scene->m_skybox->bind_prefilter_data();
    } else {
      info.scene->m_skybox->unbind_prefilter_data();
    }
    if (info.options->show_bg) {
      info.scene->m_skybox->draw(info.camera, info.options->blur);
    }
  }
  // render models
  for (const auto& model : info.scene->m_models) {
    render_meshes(model->get_meshes());
    if (info.options->show_aabb) {
      model->get_aabb().get_lines_data(m_aabb_line);
      m_shader_cache.at("lines")->use();
      RenderAPI::draw_line(m_aabb_line->vao, m_aabb_line->line_vertices.size());
    }
  }
  if (info.options->show_light_model) {
    render_meshes(info.scene->m_light_model->get_meshes());
  }
  if (info.options->show_floor) {
    render_meshes(info.scene->m_floor->get_meshes());
  }
  if (info.options->show_axis) {
    m_shader_cache.at("lines")->use();
    RenderAPI::draw_line(m_axis_line->vao, m_axis_line->line_vertices.size());
  }
}

void BasicRenderer::render_frame(const FrameInfo& info) {
  m_shadow_map->run_depth_pass(info.scene, info.options->light_type);
  set_default_state();
  m_pbuffer->bind_for_writing();
  m_pbuffer->clear();
  RenderAPI::enable_depth_testing();

  update_ubo(info);
  render_scene(info);

  m_pbuffer->unbind();

  RenderAPI::disable_depth_testing();
  RenderAPI::clear_color();

  auto& screen_shader = m_shader_cache.at("screen");
  screen_shader->use();
  if (info.options->show_depth_debug) {
    m_shadow_map->bind_debug_texture(info.options->light_type);
  } else {
    m_pbuffer->bind_for_reading("color", 0);
  }

  RenderAPI::draw_vertices(m_quad_vao, 6);
}

void BasicRenderer::resize_fbos(int width, int height) {
  m_width  = width;
  m_height = height;
  setup_framebuffers(m_width, m_height);
  glViewport(0, 0, width, height);
}
}  // namespace ezg::gl