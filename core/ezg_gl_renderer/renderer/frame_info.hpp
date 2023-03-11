#ifndef FRAME_INFO_HPP
#define FRAME_INFO_HPP
#include <glm/mat4x4.hpp>
#include "scene.hpp"
#include "systems/camera_system.hpp"
#include "engine/render_option.hpp"

namespace ezg::gl {
class ShaderProgram;
class Model;
class BaseScene;

struct ModelData {
  glm::mat4 model_matrix;
};

struct CameraData {
  glm::mat4 proj_view;
  glm::mat4 view;
  glm::mat4 projection;
};

struct FrameInfo {
  const Ref<BaseScene>& scene;
  const Ref<RenderOptions>& options;
  const Ref<system::Camera>& camera;
//  const system::Camera& camera;
};
}  // namespace ezg::gl
#endif  //FRAME_INFO_HPP
