#include "mesh.hpp"

namespace ezg::gl {
Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
    : num_vertices(vertices.size()), num_indices(indices.size()) {
  setup(vertices, indices);
}

Mesh::Mesh(const std::vector<Vertex>& vertices) : num_vertices(vertices.size()), num_indices(0) {
  setup(vertices);
}

void Mesh::setup(const std::vector<Vertex>& vertices) {
  vao = std::make_unique<SimpleVAO>();
  vao->bind();
  vao->attach_buffer(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(),
                     GL_STATIC_DRAW);
  // enable vertex attributes
  const auto vertex_size = sizeof(Vertex);
  vao->enable_attribute(0, 3, vertex_size, 0);                          // position
  vao->enable_attribute(1, 2, vertex_size, offsetof(Vertex, uv));       // texture uv
  vao->enable_attribute(2, 3, vertex_size, offsetof(Vertex, normal));   // normal
  vao->enable_attribute(3, 3, vertex_size, offsetof(Vertex, tangent));  // tangent
}

void Mesh::setup(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {
  vao = std::make_unique<SimpleIndexVAO>();
  vao->bind();
  vao->attach_buffer(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(),
                    GL_STATIC_DRAW);
  vao->attach_buffer(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(),
                    GL_STATIC_DRAW);

  // enable vertex attributes
  const auto vertex_size = sizeof(Vertex);
  vao->enable_attribute(0, 3, vertex_size, 0);                          // position
  vao->enable_attribute(1, 2, vertex_size, offsetof(Vertex, uv));       // texture uv
  vao->enable_attribute(2, 3, vertex_size, offsetof(Vertex, normal));   // normal
  vao->enable_attribute(3, 3, vertex_size, offsetof(Vertex, tangent));  // tangent
}

}  // namespace ezg::gl