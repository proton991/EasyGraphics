#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <string>
#include "assets/texture.hpp"
#include "base.hpp"
#include "graphics/shader.hpp"
#include "graphics/vertex_array.hpp"
#include "systems/camera_system.hpp"

#define DIFFUSE_RESOLUTION 256
#define SPECULAR_RESOLUTION 512

namespace ezg::gl {
class Framebuffer;
enum class SkyboxType { Cubemap, Equirectangular };
class Skybox {
public:
  static Ref<Skybox> Create(const std::vector<std::string>& face_paths);
  static Ref<Skybox> Create(const std::string& hdr_path, int resolution);
  Skybox(const std::vector<std::string>& face_paths);   // using 6 images
  Skybox(const std::string& hdr_path, int resolution);  // using hdr image
  void draw(const Ref<system::Camera>& camera, bool blur=false);
  // IBL
  void bind_prefilter_data();
  void unbind_prefilter_data();

private:
  void setup_shaders();
  void setup_screen_quads();
  void setup_cube_quads();

  void draw_cube();
  void draw_quad();
  // IBL
  void calc_prefilter_diffuse();
  void calc_prefilter_specular();
  void calc_brdf_integration();

  Ref<TextureCubeMap> m_cube_texture;
  Ref<VertexArray> m_quad_vao;
  Ref<VertexArray> m_cube_vao;

  std::unordered_map<std::string, Ref<ShaderProgram>> m_shader_cache;
  Ref<Framebuffer> m_env_fbo;
  Ref<Framebuffer> m_screen_fbo;
  SkyboxType m_type;

  int m_resolution{0};
  const unsigned int MaxMipLevels = 5;
};
}  // namespace ezg::gl
#endif  //SKYBOX_HPP
