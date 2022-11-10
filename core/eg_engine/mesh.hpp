#ifndef MESH_HPP
#define MESH_HPP

#include <glm/vec3.hpp>
#include <vector>
#include "types.hpp"

namespace ege {
struct VertexInputDescription {
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;

  VkPipelineVertexInputStateCreateFlags flags = 0;
};
struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;
  glm::vec2 uv;

  static VertexInputDescription GetVertexDescription();
};

struct Mesh {
  std::vector<Vertex> m_vertices;

  AllocatedBuffer m_vertexBuffer;

  bool LoadFromObj(const char* filename);
};
}  // namespace ege
#endif  //MESH_HPP
