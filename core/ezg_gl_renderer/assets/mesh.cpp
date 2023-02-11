#include "mesh.hpp"

namespace ezg::gl {
Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : num_vertices(vertices.size()), num_indices(indices.size()) {
  setup(vertices, indices);
}

Mesh::Mesh(const std::vector<Vertex>& vertices) : num_vertices(vertices.size()), num_indices(0) {
  setup(vertices);
}

void Mesh::setup(const std::vector<Vertex>& vertices) {
//  vao = std::make_unique<SimpleVAO>();
  vao = VertexArray::Create();
  vao->bind();
  auto vbo = VertexBuffer::Create(vertices.size() * sizeof(Vertex), vertices.data());
  vbo->set_buffer_view({
      {"aPos", BufferDataType::Vec3f},
      {"aTexCoords", BufferDataType::Vec2f},
      {"aNormal", BufferDataType::Vec3f},
  });
  vao->attach_vertex_buffer(vbo);
}

void Mesh::setup(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
  vao = VertexArray::Create();
  vao->bind();
  auto vbo = VertexBuffer::Create(vertices.size() * sizeof(Vertex), vertices.data());
  vbo->set_buffer_view({
      {"aPos", BufferDataType::Vec3f},
      {"aTexCoords", BufferDataType::Vec2f},
      {"aNormal", BufferDataType::Vec3f},
  });

  auto ibo = IndexBuffer::Create(indices.size(), indices.data());
  vao->attach_vertex_buffer(vbo);
  vao->attach_index_buffer(ibo);
}

}  // namespace ezg::gl