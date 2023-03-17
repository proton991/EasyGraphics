#include "shadow_map.hpp"
#include "engine/scene.hpp"
#include "graphics/framebuffer.hpp"
#include "log.hpp"
#include "render_api.hpp"

namespace ezg::gl {
ShadowMap::ShadowMap(uint32_t width, uint32_t height) : m_width(width), m_height(height) {
  //  AttachmentInfo depth_info{AttachmentInfo::Depth(width, height)};
  //  m_fbo    = Framebuffer::Create({width, height, {depth_info}});
  m_depth_shader = ShaderProgramFactory::create_shader_program(
      {"shadow_map_depth",
       {
           {"../resources/shaders/simple_renderer/shadowmap_depth.vs.glsl", "vertex"},
           {"../resources/shaders/simple_renderer/shadowmap_depth.fs.glsl", "fragment"},
       }});
  m_debug_shader = ShaderProgramFactory::create_shader_program(
      {"shadow_map_depth",
       {
           {"../resources/shaders/simple_renderer/debug_quad_depth.vs.glsl", "vertex"},
           {"../resources/shaders/simple_renderer/debug_quad_depth.fs.glsl", "fragment"},
       }});
  setup_framebuffer();
}

void ShadowMap::run_depth_pass(const Ref<BaseScene>& scene, const LightType& type) {
  const auto& aabb = scene->get_aabb();
  auto aabb_len    = glm::length(aabb.diag);
  // set near far plane
  m_near             = 0.01f * aabb_len;
  m_far              = 10.0f * aabb_len;
  const auto box_len = aabb_len * 2;

  glm::mat4 light_view{1.0f};
  glm::mat4 light_proj{1.0f};
  if (type == LightType::Directional) {
    light_proj = glm::ortho(-box_len, box_len, -box_len, box_len, m_near, m_far);
    // for directional light, fix the light position
    light_view =
        glm::lookAt(scene->get_aabb().bbx_max * 5.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  } else {
    light_proj = glm::perspective(glm::radians(45.0f), 1.0f, m_near, m_far);
    light_view = glm::lookAt(scene->get_light_pos(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  }
  m_light_space_mat = light_proj * light_view;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_width, m_height);
  // Clear the depth buffer of the shadow map
  glClearNamedFramebufferfv(m_fbo, GL_DEPTH, 0, &ClearDepth);
  m_depth_shader->use();
  m_depth_shader->set_uniform("uLightSpaceMat", m_light_space_mat);
  for (const auto& model : scene->m_models) {
    for (const auto& mesh : model->get_meshes()) {
      m_depth_shader->set_uniform("uModelMat", mesh.model_matrix);
      RenderAPI::draw_mesh(mesh);
    }
  }
  for (const auto& mesh : scene->m_floor->get_meshes()) {
    m_depth_shader->set_uniform("uModelMat", mesh.model_matrix);
    RenderAPI::draw_mesh(mesh);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::setup_framebuffer() {
  glCreateFramebuffers(1, &m_fbo);
  glCreateTextures(GL_TEXTURE_2D, 1, &m_depth_texture);
  glTextureParameteri(m_depth_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(m_depth_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameteri(m_depth_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTextureParameteri(m_depth_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTextureStorage2D(m_depth_texture, 1, GL_DEPTH_COMPONENT32F, m_width, m_height);
  const float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTextureParameterfv(m_depth_texture, GL_TEXTURE_BORDER_COLOR, border_color);
  glNamedFramebufferTexture(m_fbo, GL_DEPTH_ATTACHMENT, m_depth_texture, 0);
  glNamedFramebufferDrawBuffer(m_fbo, GL_NONE);
  glNamedFramebufferReadBuffer(m_fbo, GL_NONE);
  auto status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    spd::info("Framebuffer is not complete, status {}", status);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::bind_for_read(int slot) {
  glBindTextureUnit(slot, m_depth_texture);
  //  m_fbo->bind_for_reading("depth", slot);
}

void ShadowMap::bind_debug_texture(const LightType& type) {
  m_debug_shader->use();
  m_debug_shader->set_uniform("uNear", m_near);
  m_debug_shader->set_uniform("uFar", m_far);
  m_debug_shader->set_uniform("uLightType", static_cast<int>(type));
  glBindTextureUnit(0, m_depth_texture);
}
}  // namespace ezg::gl