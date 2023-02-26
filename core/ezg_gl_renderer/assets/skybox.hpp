#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <string>
#include "base.hpp"
#include "assets/texture.hpp"
#include "graphics/vertex_array.hpp"
#include "graphics/shader.hpp"
#include "systems/camera_system.hpp"

namespace ezg::gl {

class Skybox {
public:
  static Ref<Skybox> Create(const std::vector<std::string>& face_paths);
  Skybox(const std::vector<std::string>& face_paths); // using 6 images
  void draw(const system::Camera& camera);
private:
  void setup_shaders();
  void setup_screen_quads();
  void setup_cube_quads();
  void setup_cubemap_textures(const std::vector<std::string>& face_paths);
  const size_t m_resolution;
  Ref<TextureCubeMap> m_cube_texture;
  VertexArrayPtr m_quad_vao;
  VertexArrayPtr m_cube_vao;

  std::unordered_map<std::string, ShaderProgram> m_shader_cache;
  uint32_t m_texture_id{0};

};
}
#endif  //SKYBOX_HPP
