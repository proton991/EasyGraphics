#ifndef EASYGRAPHICS_RENDER_API_HPP
#define EASYGRAPHICS_RENDER_API_HPP

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace ezg::gl {
class BaseVAO;
struct Mesh;
class VertexArrayObject;
class RenderAPI {
public:
  static void set_clear_color(const glm::vec4& color);
  static void clear();
  static void enable_depth_testing();

  static void draw_vertices(const std::shared_ptr<VertexArrayObject>& vao, uint32_t num_vertices);
  static void draw_indices(const std::shared_ptr<VertexArrayObject>& vao);

  static void draw_vertices(const std::shared_ptr<BaseVAO>& vao, uint32_t num_vertices);
  static void draw_indices(const std::shared_ptr<BaseVAO>& vao, uint32_t num_indices);
  static void draw_meshes(const std::vector<Mesh>& meshes);

};

}
#endif  //EASYGRAPHICS_RENDER_API_HPP
