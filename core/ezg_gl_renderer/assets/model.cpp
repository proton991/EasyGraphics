#include "model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "log.hpp"

namespace ezg::gl {

Model::Model(const std::string& name, const std::vector<Vertex>& vertices,
             const std::vector<GLuint>& indices)
    : m_name(name) {
  m_meshes.emplace_back(vertices, indices);
  spdlog::info("Model {} loaded: {} vertices, {} indices", name, vertices.size(), indices.size());
}

Model::Model(const std::string& name, const std::vector<Vertex>& vertices) : m_name(name) {
  m_meshes.emplace_back(vertices);
}

void Model::translate(const glm::vec3& target_pos) {
  glm::vec3 delta = target_pos - m_aabb.get_center();
  for (auto& mesh : m_meshes) {
    auto translation  = glm::translate(glm::mat4(1.0), delta);
    mesh.model_matrix = translation * mesh.model_matrix;
  }

  m_aabb.translate(target_pos);
}

void Model::rotate(float angle) {
  for (auto& mesh : m_meshes) {
    mesh.model_matrix =
        glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) * mesh.model_matrix;
  }
}

void Model::scale(float factor) {
  for (auto& mesh : m_meshes) {
    const auto scale  = glm::scale(glm::mat4(1.0f), glm::vec3(factor));
    mesh.model_matrix = scale * mesh.model_matrix;
  }
  m_aabb.scale(factor);
}
}  // namespace ezg::gl