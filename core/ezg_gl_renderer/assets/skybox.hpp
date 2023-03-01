#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <string>
#include "assets/texture.hpp"
#include "base.hpp"
#include "graphics/shader.hpp"
#include "graphics/vertex_array.hpp"
#include "systems/camera_system.hpp"


namespace ezg::gl {
class Framebuffer;
enum class SkyboxType { Cubemap, Equirectangular };
class Skybox {
public:
  static Ref<Skybox> Create(const std::vector<std::string>& face_paths);
  static Ref<Skybox> Create(const std::string& hdr_path, int resolution);
  Skybox(const std::vector<std::string>& face_paths);   // using 6 images
  Skybox(const std::string& hdr_path, int resolution);  // using hdr image
  void draw(const system::Camera& camera);
  // IBL
  void bind_prefilter_data();

private:
  void setup_shaders();
  void setup_screen_quads();
  void setup_cube_quads();

  void draw_cube();
  // IBL
  void calc_prefilter_diffuse();
  void calc_prefilter_specular();
  void calc_brdf_lut();

  Ref<TextureCubeMap> m_cube_texture;
  Ref<VertexArray> m_quad_vao;
  Ref<VertexArray> m_cube_vao;

  std::unordered_map<std::string, Ref<ShaderProgram>> m_shader_cache;
  Ref<Framebuffer> m_env_fbo;
  SkyboxType m_type;

  int m_resolution{0};
};
}  // namespace ezg::gl
#endif  //SKYBOX_HPP
