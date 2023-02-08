#ifndef EASYGRAPHICS_MODEL_HPP
#define EASYGRAPHICS_MODEL_HPP

#include <memory>
#include <string>
#include "mesh.hpp"

namespace ezg::gl {
struct AABB {
  AABB() = default;
  AABB(const glm::vec3& _min, const glm::vec3& _max) : bbx_min(_min), bbx_max(_max) {
    diag = bbx_max - bbx_min;
  }
  glm::vec3 bbx_min;
  glm::vec3 bbx_max;
  glm::vec3 diag;
};
class Model;
using ModelPtr = std::shared_ptr<Model>;
class Model {
public:
  static ModelPtr Create(const std::string& name) {
    return std::make_shared<Model>(name);
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
private:
  std::string m_name;
  std::vector<Mesh> m_meshes;
  AABB m_aabb{};
};

}  // namespace ezg::gl
#endif  //EASYGRAPHICS_MODEL_HPP
