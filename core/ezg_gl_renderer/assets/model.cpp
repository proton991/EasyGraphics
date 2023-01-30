#include "model.hpp"

namespace ezg::gl {

Model::Model(const std::string& name, const std::vector<Vertex>& vertices,
             const std::vector<GLuint>& indices) : m_name(name) {
  m_meshes.emplace_back(vertices, indices);
}

Model::Model(const std::string& name, const std::vector<Vertex>& vertices) : m_name(name) {
  m_meshes.emplace_back(vertices);
}

}