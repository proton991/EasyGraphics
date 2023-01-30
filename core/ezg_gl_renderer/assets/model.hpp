#ifndef EASYGRAPHICS_MODEL_HPP
#define EASYGRAPHICS_MODEL_HPP

#include <memory>
#include <string>
#include "mesh.hpp"

namespace ezg::gl {
class Model {
public:
  Model(const std::string& name, const std::vector<Vertex>& vertices,
        const std::vector<GLuint>& indices);

  Model(const std::string& name, const std::vector<Vertex>& vertices);

private:
  const std::string& m_name;
  std::vector<Mesh> m_meshes;
};
using ModelPtr = std::shared_ptr<Model>;
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_MODEL_HPP
