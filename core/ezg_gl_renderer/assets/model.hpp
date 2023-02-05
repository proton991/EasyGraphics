#ifndef EASYGRAPHICS_MODEL_HPP
#define EASYGRAPHICS_MODEL_HPP

#include <memory>
#include <string>
#include "mesh.hpp"

namespace ezg::gl {
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
private:
  std::string m_name;
  std::vector<Mesh> m_meshes;
};

}  // namespace ezg::gl
#endif  //EASYGRAPHICS_MODEL_HPP
