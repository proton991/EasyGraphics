#ifndef EASYGRAPHICS_MODEL_HPP
#define EASYGRAPHICS_MODEL_HPP

#include <string>
#include "mesh.hpp"

namespace ezg::gl {
struct AABB {
  AABB() = default;
  AABB(const glm::vec3& _min, const glm::vec3& _max) : bbx_min(_min), bbx_max(_max) {
    diag = bbx_max - bbx_min;
  }

  [[nodiscard]] glm::vec3 get_center() const {
    return (bbx_min + bbx_max) * 0.5f;
  }

  // translate the center of the bbx to target_pos
  void translate(const glm::vec3& pos) {
    glm::vec3 delta = pos - get_center();
    bbx_min += delta;
    bbx_max += delta;
    auto new_center = get_center();
  }

  glm::vec3 bbx_min;
  glm::vec3 bbx_max;
  glm::vec3 diag;
};

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

  auto get_mesh_size() const { return m_meshes.size(); }
private:
  std::string m_name;
  std::vector<Mesh> m_meshes;
  AABB m_aabb{};
};

}  // namespace ezg::gl
#endif  //EASYGRAPHICS_MODEL_HPP
