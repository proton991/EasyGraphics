#ifndef RENDERER_DATA_HPP
#define RENDERER_DATA_HPP

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include "graphics/vertex_array.hpp"

namespace ezg::gl {
struct LineVertex {
  glm::vec3 position;
  glm::vec4 color;
};

struct CoordinateAxisData {
  std::vector<LineVertex> line_vertices;
  Ref<VertexArray> vao;
};
}  // namespace ezg::gl
#endif  //RENDERER_DATA_HPP
