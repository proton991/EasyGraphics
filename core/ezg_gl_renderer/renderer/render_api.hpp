#ifndef EASYGRAPHICS_RENDER_API_HPP
#define EASYGRAPHICS_RENDER_API_HPP

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace ezg::gl {
class Model;
struct Mesh;
class VertexArray;
class RenderAPI {
public:
  static void set_clear_color(const glm::vec4& color);
  static void clear_color_and_depth();
  static void clear_color();
  static void enable_depth_testing();
  static void disable_depth_testing();

  static void draw_vertices(const std::shared_ptr<VertexArray>& vao, uint32_t num_vertices);
  static void draw_indices(const std::shared_ptr<VertexArray>& vao);

  static void draw_meshes(const std::vector<Mesh>& meshes);
  static void draw_mesh(const Mesh& mesh);
};

}
#endif  //EASYGRAPHICS_RENDER_API_HPP
