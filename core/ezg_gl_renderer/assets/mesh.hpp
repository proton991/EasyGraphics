#ifndef EASYGRAPHICS_MESH_HPP
#define EASYGRAPHICS_MESH_HPP

#include <vector>
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "material.hpp"
#include "graphics/vertex_array.hpp"

namespace ezg::gl {
struct Vertex {
  using vec2 = glm::vec2;
  using vec3 = glm::vec3;

  Vertex() = default;

  Vertex(const vec3& position, const vec2& texture_coords)
      : position(position), uv(texture_coords) {}

  Vertex(const vec3& position, const vec2& texture_coords, const vec3& normal)
      : position(position), uv(texture_coords), normal(normal) {}

//  Vertex(const vec3& position, const vec2& uv, const vec3& normal, const vec3& tangent)
//      : position(position), uv(uv), normal(normal), tangent(tangent) {}

  vec3 position;
  vec2 uv;
  vec3 normal;
//  vec3 tangent;
};

struct Mesh {
  Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
  explicit Mesh(const std::vector<Vertex>& vertices);

  const GLsizei num_vertices;
  const GLsizei num_indices;

  Ref<VertexArray> vao;
  glm::mat4 model_matrix{1.0f};
  PBRMaterial material;

private:
  void setup(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
  void setup(const std::vector<Vertex>& vertices);


};

}  // namespace ezg::gl
#endif  //EASYGRAPHICS_MESH_HPP
