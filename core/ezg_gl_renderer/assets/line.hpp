#ifndef LINE_HPP
#define LINE_HPP
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "graphics/vertex_array.hpp"

namespace ezg::gl {
struct LineVertex {
  glm::vec3 position;
  glm::vec4 color;
};

struct Line {
  Line() = default;
  std::vector<LineVertex> line_vertices;
  Ref<VertexArray> vao;
};
}
#endif  //LINE_HPP
