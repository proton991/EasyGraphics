#ifndef EASYGRAPHICS_SHADOW_MAP_FBO_HPP
#define EASYGRAPHICS_SHADOW_MAP_FBO_HPP
#include "base.hpp"
#include "engine/render_option.hpp"
#include <glm/mat4x4.hpp>

namespace ezg::gl {
class Framebuffer;
class BaseScene;
class ShaderProgram;

class ShadowMap {
public:
  ShadowMap(uint32_t width, uint32_t height);
  void run_depth_pass(const Ref<BaseScene>& scene, const LightType& type);
  void bind_for_read(int slot);
  void bind_debug_texture();
  auto get_light_space_mat() const { return m_light_space_mat; }
private:
  void setup_framebuffer();
  uint32_t m_fbo{0};
  uint32_t m_depth_texture{0};
  uint32_t m_width{0};
  uint32_t m_height{0};
  float m_near{0.1f};
  float m_far{10.f};
  glm::mat4 m_light_space_mat;
  Ref<ShaderProgram> m_depth_shader;
  Ref<ShaderProgram> m_debug_shader;
  const float ClearDepth = 1.0f;
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_SHADOW_MAP_FBO_HPP
