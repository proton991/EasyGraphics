#ifndef FRAME_INFO_HPP
#define FRAME_INFO_HPP
#include <glm/mat4x4.hpp>
#include "systems/camera_system.hpp"
#include "scene.hpp"

namespace ezg::gl {
class ShaderProgram;
class Model;
class BaseScene;

struct ModelData {
  glm::mat4 mvp_matrix;
  glm::mat4 mv_matrix;
  glm::mat4 normal_matrix;
};

struct CameraData {
  glm::mat4 proj_view;
  glm::mat4 view;
  glm::mat4 projection;
};

struct FrameInfo {
  const BaseScenePtr& scene;
  const system::Camera& camera;
};
}
#endif  //FRAME_INFO_HPP
