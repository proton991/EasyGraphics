#ifndef EASYGRAPHICS_MODEL_HPP
#define EASYGRAPHICS_MODEL_HPP

#include <string>
#include "mesh.hpp"
#include "renderer/renderer_data.hpp"
#include "aabb.hpp"

namespace ezg::gl {
class Model {
public:
  static Ref<Model> Create(const std::string& name) {
    return CreateRef<Model>(name);
  }
  explicit Model(const std::string& name) : m_name(name) {}
  Model(const std::string& name, const std::vector<Vertex>& vertices,
        const std::vector<GLuint>& indices);

  Model(const std::string& name, const std::vector<Vertex>& vertices);
  [[nodiscard]] const std::vector<Mesh>& get_meshes() const { return m_meshes; };

  void attach_mesh(const Mesh& mesh) {
    m_meshes.push_back(mesh);
  }

  void set_aabb(const AABB& aabb) {
    m_aabb = aabb;
  }

  [[nodiscard]] const AABB& get_aabb() const {
    return m_aabb;
  }

  void translate(const glm::vec3& target_pos);
  void rotate(float angle);
  void rotate(float angle, const glm::vec3& point);
  void scale(float factor);

  auto get_mesh_size() const { return m_meshes.size(); }
  auto get_name() const { return m_name; }
private:
  std::string m_name;
  std::vector<Mesh> m_meshes;
  AABB m_aabb{};
};

}  // namespace ezg::gl
#endif  //EASYGRAPHICS_MODEL_HPP
