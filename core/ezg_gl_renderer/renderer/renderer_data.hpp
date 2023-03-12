#ifndef RENDERER_DATA_HPP
#define RENDERER_DATA_HPP

#include <glm/mat4x4.hpp>

namespace ezg::gl {
struct ModelData {
  glm::mat4 model_matrix;
};

struct CameraData {
  glm::mat4 proj_view;
  glm::mat4 view;
  glm::mat4 projection;
};
}  // namespace ezg::gl
#endif  //RENDERER_DATA_HPP
