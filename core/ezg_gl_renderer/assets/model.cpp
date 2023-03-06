#include "model.hpp"
#include "log.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace ezg::gl {

Model::Model(const std::string& name, const std::vector<Vertex>& vertices,
             const std::vector<GLuint>& indices) : m_name(name) {
  m_meshes.emplace_back(vertices, indices);
  spdlog::info("Model {} loaded: {} vertices, {} indices", name, vertices.size(), indices.size());
}

Model::Model(const std::string& name, const std::vector<Vertex>& vertices) : m_name(name) {
  m_meshes.emplace_back(vertices);
}


void Model::translate(const glm::vec3& target_pos) {
  for (const auto& mesh : m_meshes) {
    glm::translate(mesh.model_matrix, target_pos);
  }

  m_aabb.translate(target_pos);
}
}